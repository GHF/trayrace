/*
 *  Copyright (C) 2012 Xo Wang
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL XO
 *  WANG BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 *  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Except as contained in this notice, the name of Xo Wang shall not be
 *  used in advertising or otherwise to promote the sale, use or other dealings
 *  in this Software without prior written authorization from Xo Wang.
 */

#include "Renderer.h"
#include "Camera.h"
#include "Object.h"
#include "Light.h"
#include "Scene.h"

#include "embree/common/accel.h"

#include <iostream>
#include <numeric>
#include <algorithm>

namespace Trayrace {

Renderer::Renderer(size_t width, size_t height, size_t nThreads) :
                width(width),
                height(height),
                nThreads(nThreads),
                currentRow(0),
                workersRunning(true),
                workersComplete(0),
                workersAwake(false),
                scene(nullptr),
                camera(nullptr),
                pixels(nullptr) {
    for (size_t i = 0; i < nThreads; i++) {
        workers.emplace_back(&Renderer::renderThread, this);
    }
}

Renderer::~Renderer() {
    workersRunning = false;
    // wake the threads so they return
    workersAwake = true;
    workersCondVar.notify_all();
    for (std::thread &t: workers) {
        if (t.joinable()) {
            t.join();
        }
    }
}

Color Renderer::shade(const Vector3f &n, const Vector3f &wi) {
    const float c = std::max(0.f, n.dot(wi));
    return Color(c, c, c);
}

void Renderer::start(const Scene &scene, const Camera &camera, std::vector<Pixel> &pixels) {
    using namespace std;
    using namespace std::chrono;

    renderStartTime = high_resolution_clock::now();

    this->scene = &scene;
    this->camera = &camera;
    this->pixels = &pixels;
    currentRow = 0;
    workersComplete = 0;
    workersAwake = true;
    workersCondVar.notify_all();
}

void Renderer::renderThread() {
    using namespace std;
    using namespace std::chrono;

    while (workersRunning) {
        unique_lock<mutex> lock(workersMutex);
        while (!workersAwake) {
            workersCondVar.wait_for(lock, milliseconds(500));
        }
        lock.unlock();
        if (!workersRunning) {
            return;
        }

        Light::VisibilityTester visibilityTester;

        for (size_t j = currentRow++; j < height; j = currentRow++) {
            for (size_t i = 0; i < width; i++) {
                const size_t index = j * width + i;

                Ray ray = camera->generateRay(i, j);
                Hit hit;
                // trace a ray and write pixel
                scene->intersect(ray, hit);
                if (hit) {
                    const Object &obj = *scene->objects[hit.id0];
                    const Object::Face &face = obj.faces[hit.id1];

                    const auto &v0 = obj.vertices[face.vertexIdxs[0]];
                    const auto &v1 = obj.vertices[face.vertexIdxs[1]];
                    const auto &v2 = obj.vertices[face.vertexIdxs[2]];

                    const auto &ns0 = obj.normals[face.normalIdxs[0]];
                    const auto &ns1 = obj.normals[face.normalIdxs[1]];
                    const auto &ns2 = obj.normals[face.normalIdxs[2]];

                    const auto &e1 = v1 - v0;
                    const auto &e2 = v2 - v0;
                    const auto &ng = e1.cross(e2).normalized();

                    const auto &sp = BaryLerp(v0, v1, v2, hit.u, hit.v);
                    const auto &ns = BaryLerp(ns0, ns1, ns2, hit.u, hit.v).normalized();

                    Color c(0.f, 0.f, 0.f);
                    Vector3f wi;
                    for (auto lightPtr : scene->lights) {
                        const Light &light = *lightPtr;
                        float weight = 1.f / light.nSamples;
                        for (size_t i = 0; i < light.nSamples; i++) {
                            const Color lColor = light.sample(sp, EPS, wi, visibilityTester);
                            if (ng.dot(wi) > 0.f && visibilityTester.unoccluded(*scene)) {
                                c += Color(shade(ns, wi).array() * (lColor * weight).array());
                            }
                        }
                    }

                    (*pixels)[index] = Pixel(c.x(), c.y(), c.z());
//                    (*pixels)[index] = Pixel(1.f - hit.u - hit.v, hit.u, hit.v);
//                    (*pixels)[index] = Pixel(ng.x() * .5f + .5f, ng.y() * .5f + .5f, ng.z() * .5f + .5f);
                } else {
                    (*pixels)[index] = Pixel(0.f, 0.f, 0.f);
                }
            }
        }

        // the thread that finishes first sleeps all others
        workersAwake = false;

        if (++workersComplete == nThreads) {
            const time_point end = high_resolution_clock::now();
            cout << "Frame rendered in " << DurationStr(renderStartTime, end) << "." << endl;
        }
    }
}

}
