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

    e2e::spsc_queue<e2e::gp::CameraFile, e2e::constant_storage<e2e::gp::CameraFile, 1024>> jpgQueue;
    e2e::spsc_queue<e2e::LDRFrame, e2e::constant_storage<e2e::LDRFrame, 1024>> frameQueue;

    auto p = pull_frames(c, [&](auto&& frame)
    {
        jpgQueue.push(std::move(frame));
    });

    boost::optional<e2e::JpgDecoder> d;

    boost::thread decoder([&]{
        init_profiler("Decoder Thread");
        int frames = 0;

        while (jpgQueue.empty());

        auto& f = jpgQueue.front();
        d = e2e::JpgDecoder {f};
        auto& decoder = d.get();

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

    init_profiler("Main Thread");

    e2e::Window w(800, 600);

    Material hdr;
    hdr.attachShader(Material::VERTEX_SHADER, "/Users/fatih/Bitirme/samples/gl/shaders/hdr.vert");
    hdr.attachShader(Material::FRAGMENT_SHADER, "/Users/fatih/Bitirme/samples/gl/shaders/hdr.frag");
    hdr.link();

    e2e::Quad quad;
    quad.create();
    quad.set_material(hdr);

    int frames = 0;

    while (!w.ShouldClose())
    {
        if (frameQueue.empty()) continue;

        named_profile("Display")
        auto frame = std::move(frameQueue.front());
        frameQueue.pop();

        quad.addTexture(frame.buffer().data(), frame.width(), frame.height());

        w.Loop(quad);

        frames++;

        // return the frame ...
        d->return_buffer(std::move(frame));
    }

    run = false;

    decoder.join();

    print_tree();

    return 0;
}