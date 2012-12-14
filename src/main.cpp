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

#include "Object.h"
#include "Renderer.h"
#include "Transform.h"
#include "PointLight.h"
#include "AreaDiskLight.h"
#include "InteractiveCamera.h"

#include "PixelToaster/PixelToaster.h"

#include <iostream>
#include <memory>
#include <vector>

#include <random>
#include <thread>
#include <chrono>

#include <cstdlib>

int main(const int argc, const char * const argv[]) {
    using namespace Trayrace;
    using namespace PixelToaster;
    using namespace std;

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <OBJ path>" << endl;
        return EXIT_FAILURE;
    }

    const size_t width = 1024;
    const size_t height = 1024;

//    Transform look = Transform::LookAt(Vector3f(5, 2, 0), Vector3f(0, 0, 0), Vector3f(0, 1, 0));
    InteractiveCamera camera(width, height, {-.5f, .5f, -.5f, .5f}, 30.f * float(M_PI / 180.0));
    camera.theta = 10.f * float(M_PI / 180.0);
    camera.phi = M_PI_2;
    camera.r = 5.5f;
    camera.recompute();

    vector<shared_ptr<Object>> objects;
    for (size_t i = 1; i < argc; i++) {
        Object *obj = new Object(argv[i]);
        if (!obj->faces.empty()) {
            objects.emplace_back(obj);
        } else {
            delete obj;
        }
    }

    vector<shared_ptr<Light>> lights;
//    lights.emplace_back(new PointLight(Transform::Translate(2, 1.5, -2), Color(6, 6, 5.9)));
//    lights.emplace_back(new PointLight(Transform::Translate(-1, 2.5, 2), Color(5, 5, 5.1)));
    lights.emplace_back(
        new AreaDiskLight(
            Transform::LookAt(Vector3f(1, 2, 3), Vector3f(0, 0, 0), Vector3f(0, 1, 0)),
            128,
            Color(12, 12, 12),
            1.f,
            0.f));
    lights.emplace_back(
        new AreaDiskLight(
            Transform::LookAt(Vector3f(1, 2, -3), Vector3f(0, 0, 0), Vector3f(0, 1, 0)),
            128,
            Color(12, 12, 12),
            1.f,
            0.f));

    Display display("Trayrace", width, height);
    Scene scene;
    vector<Pixel> pixels(width * height);
    Renderer renderer(width, height, thread::hardware_concurrency());

    scene.build(objects, lights);
    renderer.start(scene, camera, pixels);
    display.listener(&camera);
    while (display.open()) {
        if (renderer.done()) {
            renderer.start(scene, camera, pixels);
        }
        this_thread::sleep_for(chrono::nanoseconds(1000000000 / 20));
        display.update(pixels);
    }

    return EXIT_SUCCESS;
}
