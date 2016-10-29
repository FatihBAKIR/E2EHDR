#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "gphoto_wrap/gphoto_wrapper.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>
#include <tinyformat.h>
#include <boost/thread.hpp>
#include <boost/predef.h>
#include <boost/circular_buffer.hpp>
#include <spdlog/spdlog.h>
#include <boost/lockfree/spsc_queue.hpp>
#include "jpeg_decode.h"

#if BOOST_OS_LINUX || BOOST_OS_WINDOWS
#include <GL/gl.h>
#elif BOOST_OS_MACOS
#include <OpenGL/gl.h>
#endif

#include "profiler/profiler.h"

struct thread_wrapper
{
    boost::thread t_;

    thread_wrapper(boost::thread&& t) : t_(std::move(t)) {}
    thread_wrapper(thread_wrapper&& rhs) : t_(std::move(rhs.t_)) {}
    ~thread_wrapper()
    {
        t_.interrupt();
        t_.join();
    }
};

template <class CBType>
auto pull_frames(e2e::gp::Camera& cam, CBType callback)
{
    boost::thread pull([&cam, callback] {
        init_profiler("Grabber Thread");

        {
            named_profile("Initial frame grab");
            callback(cam.liveview_frame());
        }

        while (!boost::this_thread::interruption_requested())
        {
            named_profile("Grab liveview frame");
            callback(cam.liveview_frame());
        }

        print_tree();
    });

    return thread_wrapper { std::move(pull) };
}

int main() {
    spdlog::stdout_color_mt("console");

    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();

    if (cams.size() < 1)
    {
        tfm::format(std::cerr, "No camera was found!");
        return -1;
    }

    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);

    volatile bool run = true;

    std::queue<e2e::LDRFrame> frameQueue;
    std::queue<e2e::gp::CameraFile> jpgQueue;

    auto p = pull_frames(c, [&](auto&& frame)
    {
        jpgQueue.push(std::move(frame));
    });

    boost::thread decoder([&]{
        init_profiler("Decoder Thread");
        int frames = 0;

        while (jpgQueue.empty());

        auto f = std::move(jpgQueue.front());
        jpgQueue.pop();

        e2e::JpgDecoder decoder {f};

        while (run)
        {
            if (jpgQueue.empty()) continue;

            {
                named_profile("Jpeg Decoding");
                auto file = std::move(jpgQueue.front());
                jpgQueue.pop();

                frameQueue.push(decoder.decode(file));

                frames++;
            }
        }

        print_tree();
    });

    boost::thread io([&]{
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

    decoder.join();
    io.join();

    print_tree();

    return 0;
}