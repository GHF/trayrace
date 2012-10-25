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

#ifndef MATERIALLIB_H_
#define MATERIALLIB_H_

#include "Eigen/Dense"

#include <map>
#include <string>

namespace Trayrace {

class MaterialLib {
public:
    friend class Renderer;
    struct Material {
        Eigen::Vector3f ambientColor;
        Eigen::Vector3f diffuseColor;
        Eigen::Vector3f specularColor;
        float alpha;

        Material() :
                ambientColor(), diffuseColor(), specularColor(), alpha() {
        }
    };

    static const Material *load(const std::string &path, const std::string &mtlName);

protected:
    typedef std::map<std::string, std::map<std::string, Material>> FileToNameToMaterial_t;
    static FileToNameToMaterial_t fileToNameToMtl;

    static bool parseFile(const std::string &path);
};

}

#endif /* MATERIALLIB_H_ */
