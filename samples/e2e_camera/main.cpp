#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <tinyformat.h>
#include <boost/thread.hpp>
#include <boost/predef.h>
#include <boost/circular_buffer.hpp>
#include <spdlog/spdlog.h>
#include <spsc/spsc_queue.h>
#include <Window.h>

#include <profiler/profiler.h>
#include <glsl_program.h>
#include <quad.h>
#include <texture.h>

#include <Frame.h>
#include <e2e_ff/ffmpeg_wrapper.h>

#if BOOST_OS_LINUX || BOOST_OS_WINDOWS
#include <GL/gl.h>
#elif BOOST_OS_MACOS
#include <OpenGL/gl.h>
#endif

template <class T, class Fun>
auto apply(T&& elem, Fun f)
{
    return f(std::forward<T>(elem));
}

template <class BeginT, class Fun, class... FunTs>
auto apply(BeginT&& elem, Fun f, FunTs... fs)
{
    return apply(f(std::forward<BeginT>(elem)), fs...);
};

template <class InputQueueT, class OutputQueueT, class... Rest>
auto pipeline(InputQueueT& in, OutputQueueT& out, Rest&... funs)
{
    return boost::thread(
    [&]
    {
        while (!boost::this_thread::interruption_requested())
        {
            if (in.empty()) continue;

            auto elem = std::move(in.front());
            in.pop();

            auto res = apply(elem, funs...);
            if (res)
            {
                out.emplace(std::move(*res));
            }
        }
    });
};

int main() {
    spdlog::stdout_color_mt("console");
    volatile bool run = true;

    init_profiler("Main Thread");

    e2e::Window w(1280, 360);

    std::vector<float> crf(256);
    std::iota(crf.begin(), crf.end(), 0);
    //std::transform(crf.begin(), crf.end(), crf.begin(), [](auto f){return f / 2;});

    e2e::GLSLProgram hdr;
    hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.vert");
    hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.frag");
    hdr.link();

    hdr.setUniformArray("crf", crf);

    e2e::Quad quad;
    quad.create();
    quad.set_position(-0.5, 0);
    quad.set_scale_factor(0.5, 1);
    quad.set_program(hdr);

    e2e::Quad quad1;
    quad1.create();
    quad1.set_position(0.5, 0);
    quad1.set_scale_factor(0.5, 1);
    quad1.set_program(hdr);

    auto tex1 = std::make_shared<Texture>();
    quad.set_texture(tex1);
    auto tex2 = std::make_shared<Texture>();
    quad1.set_texture(tex2);

    int frames = 0;

    using FrameT = e2e::Frame<uint8_t, 3, decltype(&av_free)>;

    e2e::ff::Camera ip_cam("rtsp://admin:admin@192.168.0.20/media/video1");
    e2e::ff::Camera ip_cam2("rtsp://admin:admin@192.168.0.21/media/video1");

    //e2e::ff::Camera ip_cam("/Users/fatih/Downloads/herb.mp4");
    boost::thread puller ([&]{ ip_cam.start_capture(); });
    boost::thread puller2 ([&]{ ip_cam2.start_capture(); });

    e2e::ff::Decoder d{ip_cam.codec_ctx_};
    e2e::ff::Decoder d2{ip_cam2.codec_ctx_};

    auto packet_handler_for = [](auto& cam, auto& d)
    {
        return [&](auto frame_data) mutable -> boost::optional<FrameT>
        {
            if(frame_data.packet->stream_index == cam.video_stream_index_)
            {
                auto ret = d.decode_one(frame_data.packet);
                av_packet_unref(frame_data.packet);

                if (ret)
                {
                    // int w, int h, e2e::ff::Decoder::data_ptr data
                    auto f = (FrameT(std::move(ret), cam.codec_ctx_->width, cam.codec_ctx_->height));
                    f.set_time(frame_data.timestamp);
                    return std::move(f);
                }
            }

            return boost::optional<FrameT>();
        };
    };

    e2e::spsc_queue<FrameT> frame_queue;
    e2e::spsc_queue<FrameT> frame_queue1;

    auto handler1 = packet_handler_for(ip_cam, d);
    auto handler2 = packet_handler_for(ip_cam2, d2);

    boost::thread decoder = pipeline(ip_cam.packet_queue, frame_queue, handler1);
    boost::thread decoder2 = pipeline(ip_cam2.packet_queue, frame_queue1, handler2);

    auto calc_diff = [](const auto& f1, const auto& f2)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds> (f1.get_time() - f2.get_time()).count();
    };

    while (!w.ShouldClose())
    {
        if (frame_queue.empty() || frame_queue1.empty()) continue;

        named_profile("Display");

        auto frame = std::move(frame_queue.front()); frame_queue.pop();
        auto frame1 = std::move(frame_queue1.front()); frame_queue1.pop();

        auto diff = calc_diff(frame1, frame);

        while (true)
        {
            if (frame1.get_time() > frame.get_time() && !frame_queue.empty())
            {
                auto di = calc_diff(frame1, frame_queue.front());
                if (std::abs(diff) > std::abs(di))
                {
                    d.return_buffer(std::move(frame.u_ptr()));
                    frame = std::move(frame_queue.front()); frame_queue.pop();
                    diff = di;
                }
                else {
                    break;
                }
            }
            else if (frame.get_time() > frame1.get_time() && !frame_queue1.empty())
            {
                auto di = calc_diff(frame, frame_queue1.front());
                if (std::abs(diff) > std::abs(di))
                {
                    d2.return_buffer(std::move(frame1.u_ptr()));
                    frame1 = std::move(frame_queue1.front()); frame_queue1.pop();
                    diff = di;
                }
                else {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        tex1->load(frame.buffer().data(), frame.width(), frame.height());
        tex2->load(frame1.buffer().data(), frame1.width(), frame1.height());
        //quad1.addTexture(frame1.buffer().data(), frame1.width(), frame1.height());

        w.Loop({quad, quad1});

        tfm::printfln("%ld ms, %d, %d", diff, frame_queue.size(), frame_queue1.size());

        frames++;

        // return the frame ...
        d.return_buffer(std::move(frame.u_ptr()));
        d2.return_buffer(std::move(frame1.u_ptr()));
    }

    run = false;

    decoder.interrupt();
    decoder.join();
    decoder2.interrupt();
    decoder2.join();

    print_tree();

    return 0;
}