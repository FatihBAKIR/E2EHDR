//
// Created by Mehmet Fatih BAKIR on 13/12/2016.
//

#include <vector>
#include <e2e_ff/ffmpeg_wrapper.h>
#include <boost/thread.hpp>
#include <texture.h>
#include <glsl_program.h>
#include <Frame.h>
#include "Application.h"
#include "configuration.h"
#include <json.hpp>
#include <Window.h>
#include <functional>
#include <merger.h>
#include <profiler/profiler.h>
#include <GLFW/glfw3.h>
#include <jpeg/jpeg_encode.h>
#include <spdlog/spdlog.h>

using FrameT = e2e::Frame<uint8_t, 3, decltype(&av_free)>;
using json = nlohmann::json;

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
auto pipeline(InputQueueT& in, OutputQueueT& out, Rest... funs)
{
    return boost::thread(
            [&, funs...]
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

class camera_struct // per camera data
{
    json                config;
    e2e::ff::Camera     camera;

    boost::thread       pull_thread;

    e2e::ff::Decoder    decoder;
    boost::thread       decode_thread;

    boost::optional<FrameT> handle_packet(e2e::ff::Camera::FrameData frame_data)
    {
        if(frame_data.packet->stream_index == camera.video_stream_index_)
        {
            auto ret = decoder.decode_one(frame_data.packet);
            av_packet_unref(frame_data.packet);

            if (ret)
            {
                // int w, int h, e2e::ff::Decoder::data_ptr data
                auto f = (FrameT(std::move(ret), camera.codec_ctx_->width, camera.codec_ctx_->height));
                f.set_time(frame_data.timestamp);
                return std::move(f);
            }
        }

        return boost::none;
    }
public:
    /// decoded frames are stored in this queue
    e2e::spsc_queue<FrameT> frame_queue;

public:

    camera_struct(const json& config) :
            config(config),
            camera((const std::string&)config["rtsp_url"]),
            decoder(camera.codec_ctx_) {}

    void start()
    {
        pull_thread = boost::thread([this]{ camera.start_capture(); });
        auto handler = [this](auto&& p) { return handle_packet(std::forward<decltype(p)>(p)); };
        decode_thread = pipeline(camera.packet_queue, frame_queue, handler);
    }

    void stop()
    {
        pull_thread.interrupt();
        decode_thread.interrupt();

        pull_thread.join();
        decode_thread.join();
    }

    void recycle(FrameT&& f)
    {
        decoder.return_buffer(f.u_ptr());
    }

    float get_exposure() const
    {
        return 1/30;
    }

    crf get_response() const
    {
        return load_crf(config["crf"]);
    }

    undistort get_undistort() const
    {
        return ::get_undistort(config);
    }

    camera_struct(camera_struct&) = delete;
    camera_struct(camera_struct&&) = delete;

    ~camera_struct()
    {
        stop();
    }
};

class ApplicationImpl
{
    camera_struct left_cam;
    camera_struct right_cam;

    struct GUI
    {
        e2e::Window w;

        Texture left_tex;
        Texture right_tex;

        e2e::Quad left_quad;
        e2e::Quad right_quad;

        GUI() : w(1280, 720)
        {
            left_quad.create();
            right_quad.create();

            left_quad.set_position(-0.5, -0.5);
            left_quad.set_scale_factor(0.5, 0.5);

            right_quad.set_position(0.5, -0.5);
            right_quad.set_scale_factor(0.5, 0.5);

            left_quad.set_texture(left_tex);

            right_quad.set_texture(right_tex);
        }
    };

    GUI gui;

    std::map<int, std::function<void()>> key_handlers;

    int snap_counter = 1;

    /*
     * This function finds the most recent image pair
     */
    std::pair<FrameT, FrameT> get_frame_pair()
    {
        auto& left_queue = left_cam.frame_queue;
        auto& right_queue = right_cam.frame_queue;

        auto left_frame = std::move(left_queue.front()); left_queue.pop();
        auto right_frame = std::move(right_queue.front()); right_queue.pop();

        auto calc_diff = [](const auto& f1, const auto& f2)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds> (f1.get_time() - f2.get_time()).count();
        };

        auto diff = calc_diff(left_frame, right_frame);

        //TODO: drop frames if too old

        while (true)
        {
            // right is newer than left and there is newer frames on the left queue
            if (right_frame.get_time() > left_frame.get_time() && !left_queue.empty())
            {
                // calculate the difference between the right frame and the more recent frame in the left queue
                auto di = calc_diff(right_frame, left_queue.front());

                // if the difference shrinks, drop left frame
                if (std::abs(diff) > std::abs(di))
                {
                    left_cam.recycle(std::move(left_frame));
                    left_frame = std::move(left_queue.front()); left_queue.pop();
                    diff = di;
                }
                else
                {
                    break;
                }
            }
            else if (left_frame.get_time() > right_frame.get_time() && !right_queue.empty())
            {
                auto di = calc_diff(left_frame, right_queue.front());
                if (std::abs(diff) > std::abs(di))
                {
                    right_cam.recycle(std::move(right_frame));
                    right_frame = std::move(right_queue.front()); right_queue.pop();
                    diff = di;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        return std::make_pair(std::move(left_frame), std::move(right_frame));
    }

    void snapshot()
    {
        std::cout << "SAVING" << '\n';

        //e2e::save_jpeg(frame.buffer().data(), frame.width(), frame.height(), "snap_210_" + std::to_string(snap_counter) + ".jpg");
        //e2e::save_jpeg(frame1.buffer().data(), frame1.width(), frame1.height(), "snap_110_" + std::to_string(snap_counter) + ".jpg");

        snap_counter++;
    }

    void reload_shaders()
    {
        std::cout << "reloading shaders...\n";

        /*hdr = e2e::GLSLProgram();

        hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.vert");
        hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.frag");
        hdr.link();

        hdr.setUniformArray("response.red", crf1.red);
        hdr.setUniformArray("response.green", crf1.green);
        hdr.setUniformArray("response.blue", crf1.blue);

        quad.set_program(hdr);
        quad1.set_program(hdr);*/
    }

    void add_keybinding(int key, std::function<void()>&& fun)
    {
        key_handlers.emplace(key, std::move(fun));
    }

    auto make_merge_shader()
    {
        e2e::GLSLProgram hdr;
        hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.vert");
        hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/merge.frag");
        hdr.link();

        auto copy_camera = [&hdr](auto& cam, const std::string& pref)
        {
            auto crf_left = cam.get_response();
            auto undis_left = cam.get_undistort();

            hdr.setUniformArray(pref + ".response.red", crf_left.red);
            hdr.setUniformArray(pref + ".response.green", crf_left.green);
            hdr.setUniformArray(pref + ".response.blue", crf_left.blue);

            hdr.setUniformFVar(pref + ".exposure", { cam.get_exposure() });

            hdr.setUniformArray(pref + ".undis.focal_length", undis_left.focal);
            hdr.setUniformArray(pref + ".undis.optical_center", undis_left.optical);
            hdr.setUniformArray(pref + ".undis.dist_coeffs", undis_left.coeffs);

            hdr.setUniformArray(pref + ".undis.image_size", {1280, 720});
        };

        copy_camera(left_cam, "left");
        copy_camera(right_cam, "right");
    }

public:
    ApplicationImpl(const std::vector <std::string> &args) :
            left_cam(load_camera_conf(args[0])),
            right_cam(load_camera_conf(args[1]))

    {
        add_keybinding(GLFW_KEY_R, [this]{
           reload_shaders();
        });

        add_keybinding(GLFW_KEY_S, [this]{
           snapshot();
        });
    }

    void Run();
};

auto make_preview_shader(const camera_struct& cam)
{
    e2e::GLSLProgram hdr;
    hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.vert");
    hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "/Users/fatih/Bitirme/samples/e2e_gl/shaders/hdr.frag");
    hdr.link();

    auto copy_camera = [&hdr](const auto& cam, const std::string& pref)
    {
        auto crf = cam.get_response();
        auto undis = cam.get_undistort();

        std::cout << "copying response\n";
        hdr.setUniformArray(pref + ".response.red", crf.red);
        hdr.setUniformArray(pref + ".response.green", crf.green);
        hdr.setUniformArray(pref + ".response.blue", crf.blue);

        hdr.setUniformFVar(pref + ".exposure", { cam.get_exposure() });

        hdr.setUniformFVar(pref + ".undis.focal_length", {undis.focal[0], undis.focal[1]});
        hdr.setUniformFVar(pref + ".undis.optical_center", {undis.optical[0], undis.optical[1]});
        hdr.setUniformArray(pref + ".undis.dist_coeffs", undis.coeffs);
        hdr.setUniformFVar(pref + ".undis.image_size", {1280, 738});
    };

    copy_camera(cam, "camera");
    return hdr;
}

void ApplicationImpl::Run()
{
    auto left_prev_shader = make_preview_shader(left_cam);
    auto right_prev_shader = make_preview_shader(right_cam);

    left_cam.start();
    right_cam.start();

    gui.left_quad.set_program(left_prev_shader);
    gui.right_quad.set_program(right_prev_shader);

    std::set<int> prev_keys;

    auto is_pressed = [&](int key) {
        return gui.w.get_key_down(key) && prev_keys.find(key) == prev_keys.end();
    };

    e2e::Merger merger { 1280, 720, 63 };
    merger.set_textures(gui.left_tex, gui.right_tex);
    merger.set_position(0.5, 0.5);
    merger.set_scale_factor(0.5, 0.5);

    while (!gui.w.ShouldClose())
    {
        if (left_cam.frame_queue.empty() || right_cam.frame_queue.empty()) continue;

        named_profile("Display");

        auto frames = get_frame_pair();
        FrameT& l_frame = std::get<0>(frames);
        FrameT& r_frame = std::get<1>(frames);

        gui.left_tex.load(l_frame.buffer().data(), l_frame.width(), l_frame.height());
        gui.right_tex.load(r_frame.buffer().data(), r_frame.width(), r_frame.height());

        auto dr1 = e2e::make_drawable(gui.left_quad);
        auto dr2 = e2e::make_drawable(gui.right_quad);
        auto dr3 = e2e::make_drawable(merger);

        gui.w.Loop({dr1, dr2});

        spdlog::get("console")->debug("%d, %d\n", left_cam.frame_queue.size(), right_cam.frame_queue.size());

        for(auto it = prev_keys.begin(); it != prev_keys.end(); )
        {
            if(gui.w.get_key_up(*it))
            {
                it = prev_keys.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (auto& keybind : key_handlers)
        {
            if (is_pressed(keybind.first))
            {
                keybind.second();
                prev_keys.emplace(keybind.first);
            }
        }

        // return the frames
        left_cam.recycle(std::move(l_frame));
        right_cam.recycle(std::move(r_frame));
    }
}


/// forwarders
Application::Application(const std::vector <std::string> &args) :
        impl(std::make_unique<ApplicationImpl>(args)){
}

void Application::Run() {
    impl->Run();
}

Application::~Application() = default;
