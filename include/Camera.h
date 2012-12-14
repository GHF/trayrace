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

#ifndef CAMERA_H_
#define CAMERA_H_

#include "Transform.h"

namespace Trayrace {

class Camera {
public:
    Transform cameraToWorld;

    Camera(const Transform &cameraToWorld, size_t xRes, size_t yRes, const float (&screenWindow)[4], float fov) :
            cameraToWorld(cameraToWorld) {
        // screen space to raster space transform
        screenToRaster = Transform::Scale(xRes, yRes, 1.f)
                * Transform::Scale(
                    1.f / (screenWindow[1] - screenWindow[0]),
                    1.f / (screenWindow[2] - screenWindow[3]),
                    1.f)
                * Transform::Translate(-screenWindow[0], -screenWindow[3], 0.f);
        rasterToScreen = screenToRaster.inverse();

        // projective camera transform: specifically for perspective
        cameraToScreen = Transform::Perspective(fov, 1e-2f, 1000.f);
        rasterToCamera = cameraToScreen.inverse() * rasterToScreen;
    }

    Ray generateRay(size_t imageX, size_t imageY) const {
        using namespace embree;
        const auto pCam = (rasterToCamera * Vector3f(imageX, imageY, 0.f)).normalized();
        Ray ray(Vec3f(0.f), Vec3f(pCam.x(), pCam.y(), pCam.z()));
        return cameraToWorld * ray;
    }

protected:
    Transform cameraToScreen;
    Transform rasterToCamera;
    Transform screenToRaster;
    Transform rasterToScreen;
};

}

#endif /* CAMERA_H_ */
