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

#ifndef INTERACTIVECAMERA_H_
#define INTERACTIVECAMERA_H_

#include "Transform.h"
#include "Trayrace.h"
#include "Camera.h"

#include "PixelToaster/PixelToaster.h"

namespace Trayrace {

class InteractiveCamera: public PixelToaster::Listener, public Camera {
public:
    float theta; // angle above xz plane
    float phi; // angle about y-axis
    float r; // distance from origin

    InteractiveCamera(
            size_t xRes,
            size_t yRes,
            const float (&screenWindow)[4],
            float fov) :
            Camera(cameraToWorld, xRes, yRes, screenWindow, fov), theta(0.f), phi(0.f), r(1.f) {
        recompute();
    }

    void recompute() {
        const Transform originTx = Transform::Rotate(phi, Vector3f(0.f, 1.f, 0.f))
                * Transform::Rotate(theta, Vector3f(-1.f, 0.f, 0.f));
        const auto origin = originTx * Vector3f(0.f, 0.f, r);
        cameraToWorld = Transform::LookAt(origin, Vector3f::Zero(), Vector3f(0.f, 1.f, 0.f));
    }

protected:
    static constexpr float ROTATE_SPEED = 1e-2f;
    static constexpr float MAX_THETA = M_PI_2 * .9f;
    static constexpr float MIN_DIST = 0.1f;
    static constexpr float MAX_DIST = 1e3f;
    static constexpr float ZOOM_RATIO = 1.1f;

    PixelToaster::Mouse lastMouse;

    virtual void onKeyPressed(PixelToaster::DisplayInterface & display, PixelToaster::Key key) {
        if (key == PixelToaster::Key::W) {
            r /= ZOOM_RATIO;
        } else if (key == PixelToaster::Key::S) {
            r *= ZOOM_RATIO;
        }
        r = Clamp(r, MIN_DIST, MAX_DIST);
        recompute();
    }

    void onMouseButtonDown(PixelToaster::DisplayInterface &display, PixelToaster::Mouse mouse) {
        if (mouse.buttons.left) {
            lastMouse = mouse;
        }
    }

    void onMouseMove(PixelToaster::DisplayInterface &display, PixelToaster::Mouse mouse) {
        if (mouse.buttons.left) {
            const float dx = mouse.x - lastMouse.x;
            const float dy = mouse.y - lastMouse.y;

            phi -= ROTATE_SPEED * dx;
            phi = ::remainderf(phi, 2.0 * M_PI);

            theta += ROTATE_SPEED * dy;
            theta = Clamp(theta, -MAX_THETA, MAX_THETA);

            recompute();

            lastMouse = mouse;
        }
    }
};

}

#endif /* INTERACTIVECAMERA_H_ */
