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

#ifndef RENDERER_H_
#define RENDERER_H_

#include "Trayrace.h"
#include "Color.h"

#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <condition_variable>

namespace Trayrace {

class Camera;
class Scene;

class Renderer {
public:
    Renderer(size_t width, size_t height, size_t nThreads);

    ~Renderer();

    void start(const Scene &scene, const Camera &camera, std::vector<Pixel> &pixels);

    bool started() const {
        return workersAwake;
    }

    bool done() const {
        return workersComplete == nThreads;
    }

protected:
    const size_t width;
    const size_t height;
    const size_t nThreads;

    std::atomic_size_t currentRow;
    time_point renderStartTime;

    // todo: this should be std::atomic_bool but libc++ doesn't have it
    std::atomic<bool> workersRunning;
    std::vector<std::thread> workers;

    // number of workers done rendering
    std::atomic_size_t workersComplete;

    // controls worker run/stop
    std::atomic<bool> workersAwake;
    std::mutex workersMutex;
    std::condition_variable workersCondVar;

    const Scene * volatile scene;
    const Camera * volatile camera;
    std::vector<Pixel> * volatile pixels;

    inline Color shade(const Vector3f &n, const Vector3f &wi);

    void renderThread();
};

}

#endif /* RENDERER_H_ */
