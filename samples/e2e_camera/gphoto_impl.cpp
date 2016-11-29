//
// Created by Mehmet Fatih BAKIR on 27/11/2016.
//

#include <gphoto/gphoto.h>
#include <boost/thread.hpp>
#include <jpeg/jpeg_decode.h>
#include <boost/optional.hpp>
#include <profiler/profiler.h>
#include <tinyformat.h>
#include <spsc/spsc_queue.h>

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

template <class CamType, class CBType>
auto pull_frames(CamType& cam, CBType callback)
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
            auto frame = cam.liveview_frame();
            callback(std::move(frame));
        }

        print_tree();
    });

    return thread_wrapper { std::move(pull) };
}

int run_gp()
{
    volatile bool run = true;

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

                auto frame = decoder.decode(file);
                frame.set_time(file.get_time());
                to.push(std::move(frame));

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

}
