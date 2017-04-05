//
// Created by Göksu Güvendiren on 28/03/2017.
//

#include <memory>
#include "Player.h"
#include <hdr_decode.hpp>

Player::Player() :
    display_window(1280, 720),
    project_window(1280, 720, nullptr, display_window.get_window())
{
    auto image2 = cv::imread("/Users/fatih/Downloads/belgium.hdr", -1);

    auto size2 = image2.size().width * image2.size().height * 3;
    auto data2 = std::make_unique<float[]>(size2);

    std::copy((const float*)image2.ptr(), (const float*)image2.ptr() + size2, data2.get());
    e2e::HDRFrame frame2(std::move(data2), image2.size().width, image2.size().height);

    frames.push(e2e::duplicate(frame2));

    pause();

    float arr[] = {0, 0, 0};
    frame_tex.createFloatBGR(1, 1, arr);

    init_shaders();
    init_quads();
    init_worker();

    //display_window.go_fullscreen(glfwGetPrimaryMonitor());
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

void Player::play_loop()
{
    while(!(display_window.ShouldClose() || project_window.ShouldClose())){
        project_window.StartDraw();

        if (is_playing.load()) {
            auto frame = std::move(Frames().front());
            Frames().pop();
            frame_tex.createFloatBGR(frame.width(), frame.height(), frame.buffer().data());
            Frames().push(std::move(frame));
        }

        prj_quad.draw();
        project_window.EndDraw();

        display_window.StartDraw();
        lcd_quad.draw();
        display_window.EndDraw();

        boost::this_thread::sleep_for(boost::chrono::milliseconds(16));
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
