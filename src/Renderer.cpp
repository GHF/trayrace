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

#include "embree/common/accel.h"

#include <iostream>
#include <numeric>
#include <algorithm>

using namespace Trayrace;

Renderer::Renderer(size_t width, size_t height) :
        width(width), height(height) {
}

void Renderer::build(const std::vector<Object> &objects) {
    using namespace embree;
    using namespace std;
    using namespace Eigen;

    const size_t numVertices = accumulate(objects.begin(),
            objects.end(),
            size_t(0),
            [&](size_t i, const Object &obj) {return i + obj.vertices_.size();});

    const size_t numTriangles = accumulate(objects.begin(),
            objects.end(),
            size_t(0),
            [&](size_t i, const Object &obj) {return i + obj.faces_.size();});

    // allocate vertex memory with embree's allocator
    BuildVertex * const vertices = (BuildVertex *) rtcMalloc(numVertices * sizeof(BuildVertex));
    // allocate faces
    BuildTriangle * const triangles = (BuildTriangle *) rtcMalloc(numTriangles * sizeof(BuildTriangle));
    // give them to Object for converting geometry to embree format
    size_t vertexOffset = 0, triangleOffset = 0;
    for (const Object &obj : objects) {
        obj.toEmbree(vertices + vertexOffset, triangles + triangleOffset);
        vertexOffset += obj.vertices_.size();
        triangleOffset += obj.faces_.size();
    }

    // create an accel structure
    Ref<Accel> accel = rtcCreateAccel("bvh4.spatialsplit",
            "triangle4i.pluecker",
            triangles,
            numTriangles,
            vertices,
            numVertices);
    // get interface to accel
    intersector = accel->queryInterface<Intersector>();
}

void Renderer::draw(std::vector<PixelToaster::Pixel> &pixels) {
    using namespace PixelToaster;
    using namespace embree;
    using namespace Eigen;

    const Pixel black(0.f, 0.f, 0.f);
    const Pixel white(1.f, 1.f, 1.f);

    // rays origin
    const Vec3f origin(278.f, 273.f, -800.f);

    const double start = timer.time();
    for (size_t j = 0; j < height; ++j) {
        for (size_t i = 0; i < width; ++i) {
            const size_t index = j * width + i;

            // scale screen coordinates to render coordinates on the xy-plane
            const float x = 556.f * float(i) / width;
            const float y = -548.8f * float(j) / height + 548.8f;

            embree::Vec3f dir(x - origin.x, y - origin.y, 0.f - origin.z);
            embree::Ray ray(origin, dir);
            embree::Hit hit;
            // trace a ray and write pixel
            intersector->intersect(ray, hit);
            if (hit) {
                MaterialLib::Material *mat = (MaterialLib::Material *) (uintptr_t(hit.id0) + (uintptr_t(hit.id1) << 32));
                if (mat != nullptr) {
                    Vector3f &c = mat->diffuseColor;
//                    cout << mat << endl;
//                    cout << &c << endl;
                    pixels[index] = (Pixel &)(c);
                }
            } else {
                pixels[index] = black;
            }
        }
    }
    const double end = timer.time();
    std::cout << "Frame rendered in " << (end - start) * 1000.0 << "ms." << std::endl;
}
