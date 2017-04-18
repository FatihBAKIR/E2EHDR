//
// Created by Göksu Güvendiren on 28/03/2017.
//

#include <memory>
#include "Player.h"
#include <hdr_decode.hpp>
#include "Video.h"
#include <nanogui/window.h>
#include <imgui_wrapper.h>
#include <gui.h>
#include <imgui.h>
#include <iostream>

Player::Player(const std::string& path) :
    display_window(1280, 720),
    project_window(1280, 720, nullptr, display_window.get_window())
{
    init_player(path);

    pause();

    float arr[] = {0.2, 0.4, 0.3};
    frame_tex.createFloatBGR(1, 1, arr);

    init_shaders();
    init_quads();
    init_worker();
    x =40;

    e2e::GUI::getGUI().initialize(display_window, true);

    init_playback();
}

void Player::init_player(const std::string& path)
{
    init_video(path);
    return;
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
    display_window.ShouldClose(true);
    project_window.ShouldClose(true);
}

void Player::play_loop()
{
    int i = 0;
    while(!(display_window.ShouldClose() || project_window.ShouldClose())){
        project_window.StartDraw();

        if (is_playing.load()) {
            if (i % 2 == 0){
                auto frame = std::move(Frames().front());
                Frames().pop();
                frame_tex.createFloatBGR(frame.width(), frame.height(), frame.buffer().data());
                Frames().push(std::move(frame));
            }
            else {
                float arr[] = {0, 0, 0};
                frame_tex.createFloatBGR(1, 1, arr);
            }
        }

        else {
            float arr[] = {0, 0, 0};
            frame_tex.createFloatBGR(1, 1, arr);
        }

        prj_quad.draw();
        project_window.EndDraw();

        display_window.StartDraw();
        lcd_quad.draw();

        draw_gui();

        display_window.EndDraw();

        i++;
        boost::this_thread::sleep_for(boost::chrono::milliseconds(x));
    }
}

void Player::init_shaders()
{
    prj_shader.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
                                   "../../e2e_gl/shaders/projector.vert");
    prj_shader.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
                                   "../../e2e_gl/shaders/projector.frag");


    lcd_shader.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
                             "../../e2e_gl/shaders/LCD.vert");
    lcd_shader.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
                             "../../e2e_gl/shaders/LCD.frag");


    lcd_shader.link();
    prj_shader.link();
}

void Player::init_quads()
{
    lcd_quad.set_scale_factor(1, 1);
    lcd_quad.set_position(0, 0);
    lcd_quad.create();
    lcd_quad.set_texture(frame_tex);

    prj_quad.set_scale_factor(1, 1);
    prj_quad.set_position(0, 0);
    prj_quad.create();
    prj_quad.set_texture(frame_tex);

    lcd_quad.set_program(lcd_shader);
    prj_quad.set_program(prj_shader);
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
    work_thread = boost::thread([this]{
        while (!boost::this_thread::interruption_requested())
        {
            if (prev_pressed.find(GLFW_KEY_SPACE) != prev_pressed.end() && (
                    display_window.get_key_up(GLFW_KEY_SPACE) || project_window.get_key_up(GLFW_KEY_SPACE)))
            {
                if (get_playing())
                {
                    pause();
                }
                else
                {
                    play();
                }
            }

            prev_pressed.clear();

            if (display_window.get_key_down(GLFW_KEY_SPACE) || project_window.get_key_down(GLFW_KEY_SPACE))
            {
                prev_pressed.insert(GLFW_KEY_SPACE);
            }
        }
    });
}

void ShowExampleMenuFile(Player& p)
{
    ImGui::MenuItem("(File)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        if (ImGui::MenuItem("output.h264")){ p.init_player("/Users/goksu/Desktop/output.h264"); }
//        ImGui::MenuItem("fish_hat.inl");
//        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();

    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        p.quit();
    }
}

void ShowProgressBar(Player& p)
{

    static float progress=0; static float progressSign = 1.f;
    std::cerr << p.get_playing() << "tessst" << std::endl;
    if (p.get_playing()) {
        progress += progressSign * .01f;
        if (progress >= 1.f || progress <= 0.f) progressSign *= -1.f;
    }
    // No IDs needed for ProgressBars:
    ImGui::ProgressBar(progress);

    if (ImGui::Button("Pause")){
        p.pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play")){
        p.play();
    }
    ImGui::SameLine();

    if (ImGui::Button("1x")){
        p.x = 40;
    }
    ImGui::SameLine();

    if (ImGui::Button("2x")){
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
    vid_thread = boost::thread([this, path]{
        e2e::x264::hdr_decode decoder(path);

        auto j = 0;
        decoder.set_handler([&](e2e::x264::decode_result&& res){
            e2e::HDRFrame f (std::make_unique<float[]>(res.residual.width() * res.residual.height() * 3),
                    res.residual.width(), res.residual.height());

            for (auto i = 0; i < res.residual.buffer().size(); ++i)
            {
                f.buffer()[i] = ((res.residual.buffer()[i] / 255.f) + res.tonemapped.buffer()[i] / 255.f) * 0.5f;
            }

            ++j;
            std::cerr << j << '\n';

            frames.push(std::move(f));
            //decoder.return_result(std::move(res));
        });

        while (!boost::this_thread::interruption_requested() && j < 100)
        {
            while (frames.size() == frames.capacity()) continue;

            decoder.decode();
        }
    });
}
