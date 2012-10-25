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

#ifndef OBJECT_H_
#define OBJECT_H_

#include "MaterialLib.h"

#include "embree/common/accel.h"

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <string>
#include <vector>

#include <stdint.h>

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix2f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3f)

namespace Trayrace {

class Object {
public:
    friend class Renderer;
    typedef int32_t IndexT;

    struct Face {
        IndexT vertexIdxs[4];
        IndexT texcoordIdxs[4];
        IndexT normalIdxs[4];
        bool isQuad;
        const MaterialLib::Material *mat;
    };

    Object(const std::string &path);
    virtual ~Object();

    void toEmbree(embree::BuildVertex *vertices, embree::BuildTriangle *triangles) const;

protected:
    std::vector<Eigen::Vector3f> vertices_;
    std::vector<Eigen::Vector2f> texcoords_;
    std::vector<Eigen::Vector3f> normals_;
    std::vector<Face> faces_;

    bool loadFile(const std::string &path);
};

}

#endif /* OBJECT_H_ */
