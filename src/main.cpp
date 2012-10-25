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

#include "Renderer.h"
#include "Object.h"
#include "PixelToaster/PixelToaster.h"

#include <iostream>
#include <vector>

#include <cstdlib>

int main(const int argc, const char * const argv[]) {
    using namespace Trayrace;
    using namespace PixelToaster;
    using namespace std;

    const size_t width = 1024;
    const size_t height = 1024;

    vector<Object> objects;
    objects.emplace_back("cornell_box.obj");

    Display display("Trayrace", width, height);
    Renderer renderer(width, height);
    renderer.build(objects);
    vector<Pixel> pixels(width * height);

    while (display.open()) {
        renderer.draw(pixels);
        display.update(pixels);
    }

    return EXIT_SUCCESS;
}
