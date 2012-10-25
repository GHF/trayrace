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

#include "MaterialLib.h"
#include "Trayrace.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace Trayrace;

MaterialLib::FileToNameToMaterial_t MaterialLib::fileToNameToMtl;

const MaterialLib::Material *MaterialLib::load(const std::string &path, const std::string &mtlName) {
    using namespace std;

    auto fileResult = fileToNameToMtl.find(path);
    if (fileResult == fileToNameToMtl.end()) {
        if (parseFile(path)) {
            fileResult = fileToNameToMtl.find(path);
        } else {
            return nullptr;
        }
    }
    auto &nameToMtl = fileResult->second;
    const auto mtl = nameToMtl.find(mtlName);
    if (mtl != nameToMtl.end()) {
        return &mtl->second;
    }
    return nullptr;
}

bool MaterialLib::parseFile(const std::string &path) {
    using namespace std;
    ifstream ifs(path.c_str(), ifstream::in);
    if (!ifs.is_open()) {
        return false;
    }

    string line;
    vector<string> tokens;
    Material *mtl = nullptr;
    auto &nameToMtl = fileToNameToMtl[path];
    while (ifs.good()) {
        tokens.clear();
        getline(ifs, line);
        istringstream iss(line);
        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(tokens));
        if (tokens.size() == 0 || tokens[0].size() == 0 || tokens[0][0] == '#')
            continue;

        if (tokens[0] == "newmtl") {
            if (tokens.size() != 2) {
                cerr << path << ": New material command does not have correct number of arguments\n";
                continue;
            }
            mtl = &nameToMtl[tokens[1]];
        } else if (mtl == nullptr) {
            cerr << path << ": Assigning material properties without a preceding \"newmtl\"\n";
            continue;
        } else if (tokens[0] == "Ka") {
            mtl->ambientColor = Eigen::Vector3f(fromString<float>(tokens[1]),
                    fromString<float>(tokens[2]),
                    fromString<float>(tokens[3]));
        } else if (tokens[0] == "Kd") {
            mtl->diffuseColor = Eigen::Vector3f(fromString<float>(tokens[1]),
                    fromString<float>(tokens[2]),
                    fromString<float>(tokens[3]));
        } else if (tokens[0] == "Ks") {
            mtl->specularColor = Eigen::Vector3f(fromString<float>(tokens[1]),
                    fromString<float>(tokens[2]),
                    fromString<float>(tokens[3]));
        } else if (tokens[0] == "d") {
            mtl->alpha = fromString<float>(tokens[1]);
        }
    }
    ifs.close();

    return true;
}
