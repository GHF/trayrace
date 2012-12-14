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

#include "AreaDiskLight.h"

#include <random>

#include <cmath>

namespace Trayrace {

static std::mt19937 generator(42);
static std::uniform_real_distribution<float> distribution(0.f, 1.f);

AreaDiskLight::AreaDiskLight(
        const Transform &lightToWorld,
        size_t nSamples,
        const Color &le,
        float radius,
        float height) :
        Light(lightToWorld, nSamples), le(le), radius(radius), height(height) {
}

Color AreaDiskLight::sample(const Vector3f &p, float pEps, Vector3f &wi, Light::VisibilityTester &vis) const {
    // todo: so much wrong here...

    // normal in light space
    Vector3f ns;
    Vector3f ps = sample(distribution(generator), distribution(generator), ns);
    // incoming (relative to shading point)
    wi = (ps - p).normalized();
    float outgoing = std::max(0.f, -ns.dot(wi));

    vis.setSegment(p, pEps, ps, 1e-3f);
    // todo: scale le according to outgoing angle
    return Color(le * (outgoing / (ps - p).squaredNorm()));
}

Color AreaDiskLight::power(const Scene &scene) const {
    // todo: this is wrong
    return Color(4.f * float(M_PI) * le);
}

static inline void UniformSampleDisk(float u1, float u2, float &x, float &y) {
    float r = ::sqrtf(u1);
    float theta = 2.f * M_PI * u2;
    x = r * ::cosf(theta);
    y = r * ::sinf(theta);
}

Vector3f AreaDiskLight::sample(float u1, float u2, Vector3f &ns) const {
    Vector3f p;
    UniformSampleDisk(u1, u2, p.x(), p.y());
    p.x() *= radius;
    p.y() *= radius;
    p.z() = height;
    ns = lightToWorld.matrix.topLeftCorner<3, 3>() * Vector3f(0.f, 0.f, 1.f);
    return lightToWorld * p;
}

}
