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

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "Trayrace.h"

namespace Trayrace {

class Transform {
public:
    Matrix4f matrix;

    Transform() :
            matrix(Matrix4f::Identity()) {
    }

    Transform(const Matrix4f &m) :
            matrix(m) {
    }

    Transform operator*(const Transform &t) const {
        return Transform(matrix * t.matrix);
    }

    Vector4f operator*(const Vector4f &v) const {
        return matrix * v;
    }

    Vector3f operator*(const Vector3f &v) const {
        Vector4f v4;
        v4 << v, 1.f;
        return (matrix * v4).head<3>();
    }

    Ray operator*(const Ray &r) const {
        using namespace embree;
        Vector4f o;
        o << Eigen::Map<const Vector3f>(&r.org.x), 1.f;
        o = matrix * o;
        const auto &d = matrix.topLeftCorner<3, 3>() * Eigen::Map<const Vector3f>(&r.dir.x);
        return Ray(Vec3f(o.x(), o.y(), o.z()), Vec3f(d.x(), d.y(), d.z()), r.near, r.far, r.time);
    }

    Transform inverse() const {
        return Transform(matrix.inverse());
    }

    Transform transpose() const {
        return Transform(matrix.transpose());
    }

    Transform translated(const float x, const float y, const float z) const {
        return Translate(x, y, z) * (*this);
    }

    Transform rotated(const float angle, const Vector3f &axis) const {
        return Rotate(angle, axis) * (*this);
    }

    Transform scaled(const float x, const float y, const float z) const {
        return Scale(x, y, z) * (*this);
    }

    static Transform Identity() {
        return Transform(Matrix4f::Identity());
    }

    static Transform Translate(const float x, const float y, const float z) {
        Transform t;
        t.matrix.topRightCorner<3, 1>() << x, y, z;
        return t;
    }

    static Transform Rotate(const float angle, const Vector3f &axis) {
        Transform t;
        t.matrix.topLeftCorner<3, 3>() = Eigen::AngleAxisf(angle, axis).matrix();
        return t;
    }

    static Transform Scale(const float x, const float y, const float z) {
        Transform t;
        t.matrix.topLeftCorner<3, 3>() = Eigen::Scaling(x, y, z);
        return t;
    }

    static Transform LookAt(const Vector3f &from, const Vector3f &to, const Vector3f &up) {
        const Vector3f forward = (to - from).normalized();
        const Vector3f side = forward.cross(up).normalized();
        const Vector3f upOrtho = side.cross(forward);

        Transform t;
        t.matrix.block<3, 1>(0, 0) = side;
        t.matrix.block<3, 1>(0, 1) = upOrtho;
        t.matrix.block<3, 1>(0, 2) = forward;
        t.matrix.block<3, 1>(0, 3) = from;
        return t;
    }

    static Transform Perspective(const float fov, const float n, const float f) {
        Matrix4f persp;
        persp << 1.f, 0.f,         0.f,              0.f,
                 0.f, 1.f,         0.f,              0.f,
                 0.f, 0.f, f / (f - n), -f * n / (f - n),
                 0.f, 0.f,         1.f,              0.f;
        const float invTanAng = 1.f / tanf(fov / 2.f);
        return Transform(Scale(invTanAng, invTanAng, 1.f).matrix * persp);
    }
};

}

#endif /* TRANSFORM_H_ */
