#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "gphoto_wrapper.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>
#include <tinyformat.h>
#include <boost/thread.hpp>
#include <boost/predef.h>

#if BOOST_OS_LINUX || BOOST_OS_WINDOWS
#include <GL/gl.h>
#elif BOOST_OS_MACOS
#include <OpenGL/gl.h>
#endif


int main() {
    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();
    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);

    volatile bool run = true;

    std::queue<CameraFile*> jpgQueue;
    std::queue<e2e::LDRFrame> frameQueue;
    boost::condition_variable cv;
    boost::mutex m;

    boost::thread framer([&]{
        int frames = 0;
        auto f = c.LiveviewFrame();
        auto frame = e2e::gp::decode(f);
        e2e::init_jpg_pool(frame.width(), frame.height());
        frameQueue.push(std::move(frame));
        e2e::gp::return_cf(f);

        auto begin = std::chrono::system_clock::now();

        while (run)
        {
            auto f = c.LiveviewFrame();
            jpgQueue.push(f);
            frames++;
            cv.notify_one();
        }

        auto end = std::chrono::system_clock::now();
        tfm::printfln("Grab FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));
    });

    boost::thread decoder([&]{
        int frames = 0;

        auto begin = std::chrono::system_clock::now();
        while (run)
        {
            if (jpgQueue.empty()) continue;

            auto file = jpgQueue.front();
            jpgQueue.pop();

            frameQueue.push(e2e::gp::decode(file));

            e2e::gp::return_cf(file);
            frames++;
        }

        auto end = std::chrono::system_clock::now();
        tfm::printfln("Decode FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));
    });

    boost::thread displayer([&]{
        std::cin.get();
        run = false;
    });

    boost::this_thread::sleep_for(boost::chrono::milliseconds(2500));

    while (run)
    {
        if (frameQueue.empty()) continue;

        auto frame = std::move(frameQueue.front());
        frameQueue.pop();

        e2e::return_buffer(std::move(frame));
    }

    framer.join();
    decoder.join();
    displayer.join();

    return 0;
}