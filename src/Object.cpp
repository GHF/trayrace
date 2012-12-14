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

#include "Object.h"
#include "Trayrace.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

#include <cstdlib>

namespace Trayrace {

Object::Object(const std::string &path) :
        path(path) {
    loadFile(path);
}

Object::~Object() {
}

static Object::IndexT indexFromStr(const std::string &s, const size_t currentIdx) {
    const Object::IndexT t = FromString<Object::IndexT>(s);
    if (t < 0) {
        return currentIdx + t + 1;
    }
    return t;
}

void Object::toEmbree(
        const int id0,
        embree::BuildVertex * const vertices,
        const size_t vertexOffset,
        embree::BuildTriangle * const triangles,
        const size_t triangleOffset) const {
    using namespace embree;
    for (size_t i = 0; i < this->vertices.size(); i++) {
        const Vector3f &v = this->vertices[i];
        // construct embree vertices in place
        new (&vertices[i + vertexOffset]) BuildVertex(v[0], v[1], v[2]);
    }
    for (size_t i = 0; i < faces.size(); i++) {
        const Face &f = faces[i];
        // construct triangles
        new (&triangles[i + triangleOffset]) BuildTriangle(f.vertexIdxs[0] + vertexOffset,
                f.vertexIdxs[1] + vertexOffset,
                f.vertexIdxs[2] + vertexOffset,
                id0,
                i);
    }
}

void Object::transformBy(const Transform &transform) {
    for (Vector3f &v : vertices) {
        v = transform * v;
    }
    const Transform normalTransform(transform.inverse().transpose());
    for (Vector3f &vn : normals) {
        vn = normalTransform * vn;
    }
}

bool Object::loadFile(const std::string &path) {
    using namespace std;
    using namespace std::chrono;

    const time_point loadStartTime = high_resolution_clock::now();

    ifstream ifs(path.c_str(), ifstream::in);
    if (!ifs.is_open()) {
        std::cerr << "File \"" << path << "\" could not be loaded." << std::endl;
        return false;
    }

    BoundBox boundBox;
    string line;
    vector<string> tokens;
    string mtlPath;
    string mtlName;
    while (ifs.good()) {
        tokens.clear();
        getline(ifs, line);
        istringstream iss(line);
        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(tokens));
        if (tokens.size() == 0 || tokens[0].size() == 0 || tokens[0][0] == '#')
            continue;

        if (tokens[0] == "v") {
            if (tokens.size() != 4) {
                cerr << path << ": Vertex command does not have correct number of components\n";
                continue;
            }
            const Vector3f vertex(FromString<float>(tokens[1]),
                    FromString<float>(tokens[2]),
                    FromString<float>(tokens[3]));
            boundBox.extend(vertex);
            vertices.push_back(vertex);
        } else if (tokens[0] == "vt") {
            if (tokens.size() != 3) {
                cerr << path << ": Texture coordinate command does not have correct number of components\n";
                continue;
            }
            const Vector2f texcoord(FromString<float>(tokens[1]), FromString<float>(tokens[2]));
            texcoords.push_back(texcoord);
        } else if (tokens[0] == "vn") {
            if (tokens.size() != 4) {
                cerr << path << ": Normal command does not have correct number of components\n";
                continue;
            }
            const Vector3f normal(FromString<float>(tokens[1]),
                    FromString<float>(tokens[2]),
                    FromString<float>(tokens[3]));
            normals.push_back(normal);
        } else if (tokens[0] == "f") {
            Face face = { };
            if (tokens.size() != 4 && tokens.size() != 5) {
                cerr << path << ": Face command does not have correct number of arguments\n";
                continue;
            }
            face.isQuad = tokens.size() == 5;
            face.mat = MaterialLib::load(mtlPath, mtlName);

            const size_t slashesCount = count(tokens[1].begin(), tokens[1].end(), '/');
            switch (slashesCount) {
            case 0: {
                for (size_t i = 0; i < 3U + face.isQuad; i++) {
                    face.vertexIdxs[i] = indexFromStr(tokens[i + 1], vertices.size()) - 1;
                }
                faces.push_back(face);
                break;
            }

            case 1: {
                for (size_t i = 0; i < 3U + face.isQuad; i++) {
                    const std::string &token = tokens[i + 1];
                    const size_t slashPos = token.find('/');
                    face.vertexIdxs[i] = indexFromStr(token.substr(0, slashPos), vertices.size()) - 1;
                    if (token.size() - slashPos > 1)
                        face.texcoordIdxs[i] = indexFromStr(token.substr(slashPos + 1), texcoords.size()) - 1;
                }
                faces.push_back(face);
                break;
            }

            case 2: {
                for (size_t i = 0; i < 3U + face.isQuad; i++) {
                    const std::string &token = tokens[i + 1];
                    const size_t slash0Pos = token.find('/');
                    const size_t slash1Pos = token.find('/', slash0Pos + 1);
                    face.vertexIdxs[i] = indexFromStr(token.substr(0, slash0Pos), vertices.size()) - 1;
                    if (slash1Pos - slash0Pos > 1)
                        face.texcoordIdxs[i] = indexFromStr(token.substr(slash0Pos + 1, slash1Pos - slash0Pos),
                                texcoords.size()) - 1;
                    if (token.size() - slash0Pos > 1)
                        face.normalIdxs[i] = indexFromStr(token.substr(slash1Pos + 1), normals.size()) - 1;
                }
                faces.push_back(face);
                break;
            }

            default:
                cerr << path << ": Invalid face command in \"" << line << "\"\n";
                break;
            }
        } else if (tokens[0] == "usemtl") {
            if (tokens.size() != 2) {
                cerr << path << ": Use material command does not have correct number of arguments\n";
                continue;
            }
            mtlName = tokens[1];
        } else if (tokens[0] == "mtllib") {
            if (tokens.size() != 2) {
                cerr << path << ": Material library command does not have correct number of arguments\n";
                continue;
            }
            mtlPath = tokens[1];
        } else {
            cerr << path << ": Unrecognized token \"" << tokens[0] << "\"\n";
        }
    }
    ifs.close();

    const time_point loadEndTime = high_resolution_clock::now();
    cout
            << "File \""
            << path
            << "\" loaded in "
            << DurationStr(loadStartTime, loadEndTime)
            << " with "
            << faces.size()
            << " face(s), "
            << normals.size()
            << " normal(s), and "
            << texcoords.size()
            << " texture coordinate(s).\n"
            << "\tBounding box: {"
            << boundBox.min().transpose()
            << ", "
            << boundBox.max().transpose()
            << "}\n";

    return true;
}

}
