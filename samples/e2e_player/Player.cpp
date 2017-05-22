//
// Created by Göksu Güvendiren on 28/03/2017.
//

#include <memory>
#include "Player.h"
#include <hdr_decode.hpp>
#include "Video.h"
#include <imgui_wrapper.h>
#include <gui.h>
#include <imgui.h>
#include <iostream>
#include "shared_frame_queue.hpp"
#include <util.h>

Player::Player(short w, short h) :
    m_width(w), m_height(h)
{
}

void Player::init_player(std::function<void()> preplay)
{
    x = 40;

    prj_quad.set_vertices(
            {-1.05999994, 1.23499978, 0, 1, -1.07999992, -0.865000129, 0, 0, 1.02499998, -0.910000086, 1, 0, 1.05499995,
             1.17999983, 1, 1});
    pause();
    display_window = new e2e::Window(m_width, m_height);
    project_window = new e2e::Window(m_width, m_height, nullptr, display_window->get_window());
    e2e::GUI::getGUI().initialize(*display_window, true);

    float arr[] = {0.2, 0.4, 0.3};
    frame_tex.createFloatBGR(1, 1, arr);

    init_shaders();
    init_quads();
    init_worker();

    int num;
    //display_window->go_fullscreen(glfwGetMonitors(&num)[1]);
    //project_window->go_fullscreen(glfwGetMonitors(&num)[2]);

    preplay();
    init_playback();
}

void Player::init_playback()
{
    play_loop();
}

void Player::play()
{
    is_playing.store(true);
}

void Player::pause()
{
    is_playing.store(false);
}

void Player::quit()
{
    display_window->ShouldClose(true);
    project_window->ShouldClose(true);
}

void Player::play_loop()
{
    int i = 0;
    while (!(display_window->ShouldClose() || project_window->ShouldClose())) {
        if (is_playing.load()) {
            prj_shader.setUniformFVar("im_scale", { im_scale_x, im_scale_y });
            lcd_shader.setUniformFVar("im_scale", { im_scale_x, im_scale_y });

            int is_ldr = ldr_mode.load();
            prj_shader.use();
            prj_shader.setUniformIVar("ldr_mode", { is_ldr });
            lcd_shader.use();
            lcd_shader.setUniformIVar("ldr_mode", { is_ldr });

            boost::apply_visitor(
                e2e::make_lambda_visitor<void>(
                    [this](half_frames_t& frames) {
                        auto frame = std::move(frames->front());
                        frames->pop();
                        frame_tex.createHalf(frame.width(), frame.height(), frame.buffer().data());
                        frames->push(std::move(frame));
                    }, [this](float_frames_t& frames) {
                        auto frame = std::move(frames->front());
                        frames->pop();
                        frame_tex.createFloatBGR(frame.width(), frame.height(), frame.buffer().data());
                        frames->push(std::move(frame));
                    }), frames);
        }
        else {
            float arr[] = {0, 0, 0};
            frame_tex.createFloatBGR(1, 1, arr);
        }

        project_window->StartDraw();
        prj_quad.draw();
        project_window->EndDraw();

        display_window->StartDraw();
        lcd_quad.draw();
        //draw_gui();
        display_window->EndDraw();

        i++;

        if (display_window->get_key_down(GLFW_KEY_DOWN) || project_window->get_key_down(GLFW_KEY_DOWN))
            prj_quad.updateVertex(corner_id, 0, 0.005f);
        if (display_window->get_key_down(GLFW_KEY_UP) || project_window->get_key_down(GLFW_KEY_UP))
            prj_quad.updateVertex(corner_id, 0, -0.005f);
        if (display_window->get_key_down(GLFW_KEY_RIGHT) || project_window->get_key_down(GLFW_KEY_RIGHT))
            prj_quad.updateVertex(corner_id, -0.005f, 0);
        if (display_window->get_key_down(GLFW_KEY_LEFT) || project_window->get_key_down(GLFW_KEY_LEFT))
            prj_quad.updateVertex(corner_id, 0.005f, 0);

        if (display_window->get_key_down(GLFW_KEY_1) || project_window->get_key_down(GLFW_KEY_1))
            corner_id = 2;

        if (display_window->get_key_down(GLFW_KEY_2) || project_window->get_key_down(GLFW_KEY_2))
            corner_id = 1;

        if (display_window->get_key_down(GLFW_KEY_3) || project_window->get_key_down(GLFW_KEY_3))
            corner_id = 0;

        if (display_window->get_key_down(GLFW_KEY_4) || project_window->get_key_down(GLFW_KEY_4))
            corner_id = 3;

        boost::this_thread::sleep_for(boost::chrono::milliseconds(x));
    }
}

void Player::init_shaders()
{
    prj_shader.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
            "../shaders/projector.vert");
    prj_shader.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
            "../shaders/projector.frag");
    /*prj_shader.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
                            "../shaders/checkerboard.frag");*/

    lcd_shader.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
            "../shaders/LCD.vert");
    lcd_shader.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
            "../shaders/LCD.frag");

    lcd_shader.link();
    prj_shader.link();
}

void Player::init_quads()
{
    display_window->StartDraw();
    lcd_quad.set_scale_factor(1, 1);
    lcd_quad.set_position(0, 0);
    lcd_quad.create();
    lcd_quad.set_texture(frame_tex);
    lcd_quad.set_program(lcd_shader);

    display_window->EndDraw();

    project_window->StartDraw();
    prj_quad.set_scale_factor(1, 1);
    prj_quad.set_position(0, 0);
    prj_quad.create();
    prj_quad.set_texture(frame_tex);
    prj_quad.set_program(prj_shader);

    project_window->EndDraw();
}

Player::~Player()
{
    work_thread.interrupt();
    work_thread.join();

    vid_thread.interrupt();
    vid_thread.join();
}

void Player::init_worker()
{
    work_thread = boost::thread([this] {
        while (!boost::this_thread::interruption_requested()) {
            if (prev_pressed.find(GLFW_KEY_SPACE)!=prev_pressed.end() && (
                    display_window->get_key_up(GLFW_KEY_SPACE) || project_window->get_key_up(GLFW_KEY_SPACE))) {
                if (get_playing()) {
                    pause();
                }
                else {
                    play();
                }
            }

            prev_pressed.clear();

            if (display_window->get_key_down(GLFW_KEY_SPACE) || project_window->get_key_down(GLFW_KEY_SPACE)) {
                prev_pressed.insert(GLFW_KEY_SPACE);
            }
        }
    });
}

void ShowExampleMenuFile(Player& p)
{
    ImGui::MenuItem("(File)", NULL, false, false);
    if (ImGui::MenuItem("New")) { }
    if (ImGui::MenuItem("Open", "Ctrl+O")) { }
    if (ImGui::BeginMenu("Open Recent")) {
        if (ImGui::MenuItem("output.h264")) { p.load_media(video{}, "/Users/goksu/Desktop/output.h264", 1280, 720); }
//        ImGui::MenuItem("fish_hat.inl");
//        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More..")) {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) { }
    if (ImGui::MenuItem("Save As..")) { }

    ImGui::Separator();

    if (ImGui::BeginMenu("Options")) {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i<10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Checked", NULL, true)) { }
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        p.quit();
    }
}

void ShowProgressBar(Player& p)
{

    static float progress = 0;
    static float progressSign = 1.f;
    std::cerr << p.get_playing() << "tessst" << std::endl;
    if (p.get_playing()) {
        progress += progressSign*.01f;
        if (progress>=1.f || progress<=0.f) progressSign *= -1.f;
    }
    // No IDs needed for ProgressBars:
    ImGui::ProgressBar(progress);

    if (ImGui::Button("Pause")) {
        p.pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play")) {
        p.play();
    }
    ImGui::SameLine();

    if (ImGui::Button("1x")) {
        p.x = 40;
    }
    ImGui::SameLine();

    if (ImGui::Button("2x")) {
        p.x = (p.x)/2;
    }
}

void Player::draw_gui()
{
    e2e::GUI::getGUI().newFrame();
    ImGui::Begin("");

    ShowExampleMenuFile(*this);

    ImGui::End();

    ImGui::Begin("Progress Bar");
    ShowProgressBar(*this);
    ImGui::End();
    ImGui::Render();
}

void Player::init_video(const std::string& path)
{
    vid_thread = boost::thread([this, path] {
        auto j = 0;
/*

        e2e::x264::hdr_decode decoder(path, 1280, 720);

        decoder.set_handler([&](e2e::x264::decode_result&& res){
            */
/*e2e::HDRFrame f (std::make_unique<float[]>(res.residual.width() * res.residual.height() * 3),
                    res.residual.width(), res.residual.height());*//*


            auto buf = std::make_unique<uint16_t[]>(res.residual.width() * res.residual.height() * 3);

            gsl::span<uint16_t> first_half = {reinterpret_cast<uint16_t*>(res.tonemapped.buffer().data()),
                                              res.tonemapped.buffer().size() / 2 };

            gsl::span<uint16_t> second_half = {reinterpret_cast<uint16_t*>(res.residual.buffer().data()),
                                              res.residual.buffer().size() / 2 };

            std::copy(first_half.begin(), first_half.end(), buf.get());
            std::copy(second_half.begin(), second_half.end(), buf.get() + first_half.size());

            ++j;
            std::cerr << j << '\n';

            auto f = e2e::HalfFrame(std::move(buf), res.residual.width(), res.residual.height());

            frames.push(std::move(f));
            decoder.return_result(std::move(res));
        });
*/

        auto decode = [&] {
            std::ifstream input("test"+std::to_string(j)+".bin", std::ios::binary);

            j++;
            auto f = e2e::HalfFrame(std::make_unique<uint16_t[]>(1280*720*3), 1280, 720);
            input.read((char*) f.buffer().data(), f.buffer().size_bytes());
            boost::get<half_frames_t>(frames)->push(std::move(f));
        };

        while (!boost::this_thread::interruption_requested() && j<379) {
            while (boost::apply_visitor([](auto& f) {
                return f->size()==f->capacity();
            }, frames))
                continue;

            decode();
            //decoder.decode();
        }

        std::cerr << "done\n";
    });
    vid_thread.join();
}

void Player::init_ipc()
{
    auto fs = mq.receive();

    e2e::HDRFrame f(std::make_unique<float[]>(fs.res.width()*fs.res.height()*3),
            fs.res.width(), fs.res.height());

    for (auto i = 0; i<fs.res.buffer().size(); ++i) {
        f.buffer()[i] = ((fs.res.buffer()[i]/255.f)+fs.tmo.buffer()[i]/255.f)*0.5f;
    }

    //frames.push(std::move(f));
}

void Player::load_media(video, const std::string& path, short w, short h)
{
    pause();
    frames = std::make_unique<typename half_frames_t::element_type>();

    init_video(path);
}

void Player::load_media(image, const std::string& path)
{
    pause();
    frames = std::make_unique<typename float_frames_t::element_type>();
    auto image2 = cv::imread(path, -1);

    auto size2 = image2.size().width*image2.size().height*3;
    auto data2 = std::make_unique<float[]>(size2);

    std::copy((const float*) image2.ptr(), (const float*) image2.ptr()+size2, data2.get());
    e2e::HDRFrame frame2(std::move(data2), image2.size().width, image2.size().height);

    boost::get<float_frames_t>(frames)->push(e2e::duplicate(frame2));

    auto res = calculate_size({image2.size().width, image2.size().height});
    im_scale_x = (float)res.first / m_width;
    im_scale_y = (float)res.second / m_height;

    //lcd_quad.set_scale_factor(x_scale, y_scale);
    //prj_quad.set_scale_factor(x_scale, y_scale);
}

std::pair<short, short> Player::calculate_size(const std::pair<short, short>& res) const
{
    float img_width = res.first;
    float img_height = res.second;

    auto w = img_width;
    auto h = img_height;

    auto width_ratio = m_width / w;

    if (width_ratio < 1){
        w = w * width_ratio;
        h = h * width_ratio;
    }

    auto height_ratio = m_height / h;
    if (height_ratio < 1){
        h = h * height_ratio;
        w = w * height_ratio;
    }

    return {w, h};
}

void Player::set_ldr_mode(bool ldr)
{
    ldr_mode.store(ldr);
}
