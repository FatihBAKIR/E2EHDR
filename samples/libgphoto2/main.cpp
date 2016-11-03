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
#include <boost/optional.hpp>
#include "spsc/spsc_queue.h"
#include <pipeline.h>
#include <quad.h>
#include <Window.h>

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

    if (cams.size() < 2)
    {
        tfm::format(std::cerr, "No camera was found!");
        return -1;
    }

    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);
    e2e::gp::Camera c1 (cams[1], gp);

    volatile bool run = true;

    e2e::spsc_queue<e2e::gp::CameraFile, e2e::constant_storage<e2e::gp::CameraFile, 1024>> jpgQueue;
    e2e::spsc_queue<e2e::gp::CameraFile, e2e::constant_storage<e2e::gp::CameraFile, 1024>> jpgQueue1;
    e2e::spsc_queue<e2e::LDRFrame, e2e::constant_storage<e2e::LDRFrame, 1024>> frameQueue;
    e2e::spsc_queue<e2e::LDRFrame, e2e::constant_storage<e2e::LDRFrame, 1024>> frameQueue1;

    auto p = pull_frames(c, [&](auto&& frame)
    {
        jpgQueue.push(std::move(frame));
    });

    auto p2 = pull_frames(c1, [&](auto&& frame)
    {
        jpgQueue1.push(std::move(frame));
    });

    boost::optional<e2e::JpgDecoder> d;
    boost::optional<e2e::JpgDecoder> d1;

    boost::thread decoder([&]{
        init_profiler("Decoder Thread");
        int frames = 0;

        auto cam = [&](auto& queue, auto& decoder, auto& to){
            if (queue.empty()) return;
            {
                named_profile("Jpeg Decoding");
                auto file = std::move(queue.front());
                queue.pop();

                to.push(decoder.decode(file));

                frames++;
            }
        };

        while (jpgQueue.empty());
        auto& f = jpgQueue.front();
        d = e2e::JpgDecoder {f};
        auto& decoder = d.get();

        while (jpgQueue1.empty());
        auto& f1 = jpgQueue1.front();
        d1 = e2e::JpgDecoder {f1};
        auto& decoder1 = d1.get();

        while (run)
        {
            cam(jpgQueue, decoder, frameQueue);
            cam(jpgQueue1, decoder1, frameQueue1);
        }

        print_tree();
    });

    init_profiler("Main Thread");

    e2e::Window w(800, 600);

    Material hdr;
    hdr.attachShader(Material::VERTEX_SHADER, "/home/fatih/E2EHDR/samples/gl/shaders/hdr.vert");
    hdr.attachShader(Material::FRAGMENT_SHADER, "/home/fatih/E2EHDR/samples/gl/shaders/hdr.frag");
    hdr.link();

    e2e::Quad quad;
    quad.create();
    quad.set_position(-0.5, 0);
    quad.set_scale_factor(0.5, 0.5);
    quad.set_material(hdr);

    e2e::Quad quad1;
    quad1.create();
    quad1.set_position(0.5, 0);
    quad1.set_scale_factor(0.5, 0.5);
    quad1.set_material(hdr);

    int frames = 0;

    while (!w.ShouldClose())
    {
        if (frameQueue.empty() || frameQueue1.empty()) continue;

        named_profile("Display")
        auto frame = std::move(frameQueue.front());
        frameQueue.pop();

        auto frame1 = std::move(frameQueue1.front());
        frameQueue1.pop();

        quad.addTexture(frame.buffer().data(), frame.width(), frame.height());
        quad1.addTexture(frame1.buffer().data(), frame1.width(), frame1.height());

        w.Loop({quad, quad1});

        frames++;

        // return the frame ...
        d->return_buffer(std::move(frame));
        d1->return_buffer(std::move(frame1));
    }

    run = false;

    decoder.join();

    print_tree();

    return 0;
}