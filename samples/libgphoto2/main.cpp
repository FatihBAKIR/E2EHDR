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
#include <boost/circular_buffer.hpp>
#include <spdlog/spdlog.h>

#if BOOST_OS_LINUX || BOOST_OS_WINDOWS
#include <GL/gl.h>
#elif BOOST_OS_MACOS
#include <OpenGL/gl.h>
#endif

#include "profiler.h"

int main() {
    spdlog::stdout_color_mt("console");

    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();
    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);

    volatile bool run = true;

    boost::circular_buffer<e2e::LDRFrame> buf(64);
    boost::circular_buffer<CameraFile*> buf2(64);

    std::queue<CameraFile*, boost::circular_buffer<CameraFile*>> jpgQueue (buf2);
    std::queue<e2e::LDRFrame, boost::circular_buffer<e2e::LDRFrame>> frameQueue (std::move(buf));

    boost::thread framer([&]{
        init_profiler("Grabber Thread");

        int frames = 0;
        auto f = c.LiveviewFrame();
        auto frame = e2e::gp::decode(f);
        e2e::init_jpg_pool(frame.width(), frame.height());
        frameQueue.push(std::move(frame));
        e2e::gp::return_cf(f);

        while (run)
        {
            named_profile("Grab liveview frame");
            auto f = c.LiveviewFrame();
            jpgQueue.push(f);
            frames++;
        }

        print_tree();
    });

    boost::thread decoder([&]{
        init_profiler("Decoder Thread");
        int frames = 0;

        while (run)
        {
            if (jpgQueue.empty()) continue;

            {
                named_profile("Jpeg Decoding");
                auto file = jpgQueue.front();
                jpgQueue.pop();

                frameQueue.push(e2e::gp::decode(file));

                e2e::gp::return_cf(file);
                frames++;
            }
        }

        print_tree();
    });

    boost::thread displayer([&]{
        std::cin.get();
        run = false;
    });

    boost::this_thread::sleep_for(boost::chrono::milliseconds(2500));

    init_profiler("Main Thread");
    int frames = 0;
    while (run)
    {
        if (frameQueue.empty()) continue;

        {
            named_profile("Display")
            auto frame = std::move(frameQueue.front());
            frameQueue.pop();

            /*cv::Mat img (cv::Size(frame.width(), frame.height()), CV_8UC3, frame.buffer().data());
            cv::cvtColor(img, img, CV_RGB2BGR);
            cv::imshow("hai", img);
            cv::waitKey(1);*/

            frames++;

            e2e::return_buffer(std::move(frame));
        }
    }

    framer.join();
    decoder.join();
    displayer.join();

    print_tree();

    return 0;
}