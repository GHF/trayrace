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

#ifndef LIGHT_H_
#define LIGHT_H_

#include "Color.h"
#include "Scene.h"
#include "Trayrace.h"
#include "Transform.h"

#include <ostream>
#include <algorithm>

namespace Trayrace {

class Light {
public:
    const size_t nSamples;

    class VisibilityTester {
    public:
        Ray ray;

        void setSegment(const Vector3f &p1, float eps1, const Vector3f &p2, float eps2) {
            const float dist = (p1 - p2).norm();
            ray = Ray(EmbV(p1), EmbV((p2 - p1) / dist), eps1, dist * (1.f - eps2));
        }

        void setRay(const Vector3f &p, float eps, const Vector3f &w) {
            ray = Ray(EmbV(p), EmbV(w), eps);
        }

        bool unoccluded(const Scene &scene) const {
            return !scene.occluded(ray);
        }
    };

    Light(const Transform &lightToWorld, size_t nSamples = 1) :
                    nSamples(std::max(nSamples, decltype(nSamples)(1))),
                    lightToWorld(lightToWorld),
                    worldToLight(lightToWorld.inverse()) {
    }

    virtual ~Light() {
    }

    virtual Color sample(const Vector3f &p, float pEps, Vector3f &wi, VisibilityTester &vis) const = 0;

    virtual Color power(const Scene &scene) const = 0;

    virtual bool isDeltaLight() const = 0;

    friend std::ostream &operator<<(std::ostream &os, const Light &l) {
        return os << "Light { nSamples = "
                << l.nSamples
                << ", lightToWorld = \n"
                << l.lightToWorld.matrix
                << ", worldToLight = \n"
                << l.worldToLight.matrix
                << " }";
    }

protected:
    Transform lightToWorld;
    Transform worldToLight;
};

}

#endif /* LIGHT_H_ */
