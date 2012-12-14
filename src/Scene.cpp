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

#include "Scene.h"
#include "Light.h"
#include "Object.h"

#include "embree/common/accel.h"

#include <iostream>
#include <numeric>
#include <algorithm>

namespace Trayrace {

Scene::Scene() {
}

void Scene::build(const std::vector<std::shared_ptr<Object>> &objects, const std::vector<std::shared_ptr<Light>> &lights) {
    using namespace embree;
    using namespace std;

    this->objects = objects;
    this->lights = lights;

    const size_t numVertices = accumulate(objects.begin(),
            objects.end(),
            size_t(0),
            [&](size_t i, shared_ptr<Object> obj) {return i + obj->vertices.size();});

    const size_t numTriangles = accumulate(objects.begin(),
            objects.end(),
            size_t(0),
            [&](size_t i, shared_ptr<Object> obj) {return i + obj->faces.size();});

    // allocate vertex memory with embree's allocator
    BuildVertex * const vertices = (BuildVertex *) rtcMalloc(numVertices * sizeof(BuildVertex));
    // allocate faces
    BuildTriangle * const triangles = (BuildTriangle *) rtcMalloc(numTriangles * sizeof(BuildTriangle));
    // give them to Object for converting geometry to embree format
    size_t vertexOffset = 0, triangleOffset = 0;
    for (size_t i = 0; i < objects.size(); i++) {
        const Object &obj = *objects[i];
        obj.toEmbree(i, vertices, vertexOffset, triangles, triangleOffset);
        vertexOffset += obj.vertices.size();
        triangleOffset += obj.faces.size();
    }

    cout << "Calling embree build with " << numTriangles << " tris and " << numVertices << " verts..." << endl;

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

}
