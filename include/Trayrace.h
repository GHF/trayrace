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

#ifndef TRAYRACE_H_
#define TRAYRACE_H_

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <embree/common/ray.h>
#include <embree/common/hit.h>

#include <PixelToaster/PixelToaster.h>

#include <array>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>

#define TR_ALIGN(x) __attribute__((__aligned__(x))) // todo: change to alignas(x) when Eclipse is less dumb

namespace Trayrace {

template<typename T>
inline T FromString(const std::string &s) {
    std::istringstream iss(s);
    T t;
    iss >> t;
    return t;
}

template<typename T>
inline std::string ToString(const T &t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

template<typename Scalar, typename T1, typename T2>
inline Scalar Clamp(Scalar s, T1 min, T2 max) {
    return std::min(std::max(s, Scalar(min)), Scalar(max));
}

template<typename T, typename Scalar>
inline auto BaryLerp(const T &v0, const T &v1, const T &v2, Scalar u, Scalar v) -> decltype(v1 * u + v2 * v + v0 * (Scalar(1.0) - u - v)) {
    return v1 * u + v2 * v + v0 * (Scalar(1.0) - u - v);
}

constexpr float EPS = 1e-4f;

typedef Eigen::Vector2f Vector2f;
typedef Eigen::Vector3f Vector3f;
typedef Eigen::Vector4f Vector4f;
typedef Eigen::Matrix4f Matrix4f;
typedef Eigen::AlignedBox3f BoundBox;

typedef embree::Ray Ray;
typedef embree::Hit Hit;

inline embree::Vec3f EmbV(const Vector3f &v) {
    embree::Vec3f embV;
    Eigen::Map<Vector3f>(&embV.x) = v;
    return embV;
}

typedef PixelToaster::Pixel Pixel;
typedef PixelToaster::Timer Timer;

typedef std::chrono::high_resolution_clock::time_point time_point;

inline std::string DurationStr(time_point start, time_point end) {
    using namespace std;
    using namespace std::chrono;
    const auto length = end - start;
    const array<time_point::duration::rep, 6> lengths = { {
            duration_cast<hours>(length).count(),
            duration_cast<minutes>(length).count(),
            duration_cast<seconds>(length).count(),
            duration_cast<milliseconds>(length).count(),
            duration_cast<microseconds>(length).count(),
            duration_cast<nanoseconds>(length).count() } };
    const array<string, 6> units = { { "hr", "min", "s", "ms", "us", "ns" } };
    const decltype(lengths) periods = { { 60, 60, 1000, 1000, 1000, 1000 } };

    time_point::duration::rep modulo = 0;
    string output;
    for (size_t i = 0; i < lengths.size(); i++) {
        const auto remainder = lengths[i] - modulo;
        if (lengths[i] != 0) {
            output += ToString(remainder) + " " + units[i];
            if (i == 5 || lengths[i] >= 100) {
                break;
            }
            output += " ";
        }
        modulo = lengths[i] * periods[i];
    }
    return output;
}

}

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix2f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix4f)


#endif /* TRAYRACE_H_ */
