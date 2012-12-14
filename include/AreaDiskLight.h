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

#ifndef AREADISKLIGHT_H_
#define AREADISKLIGHT_H_

#include "Light.h"

namespace Trayrace {

class AreaDiskLight: public Light {
public:
    Color le;
    float radius;
    float height;

    AreaDiskLight(const Transform &lightToWorld, size_t nSamples, const Color &le, float radius, float height);

    Color sample(const Vector3f &p, float pEps, Vector3f &wi, Light::VisibilityTester &vis) const;

    Color power(const Scene &scene) const;

    bool isDeltaLight() const {
        return false;
    }

    Vector3f sample(float u1, float u2, Vector3f &ns) const;

    // todo: this should be shading point and light w_i, not light point
    float pdf(const Vector3f &p, const Vector3f &lp) const {
        // todo
        return 1.f;
    }

    bool intersect(const Ray &ray, float &tHit, float &rayEpsilon) const {
        // todo
        return false;
    }

    friend std::ostream &operator<<(std::ostream &os, const AreaDiskLight &dl) {
        return os << "AreaDiskLight { "
                << (Light &) dl
                << ", intensity = "
                << dl.le.transpose()
                << ", radius = "
                << dl.radius
                << ", height = "
                << dl.height
                << " }";
    }
};

}

#endif /* AREADISKLIGHT_H_ */
