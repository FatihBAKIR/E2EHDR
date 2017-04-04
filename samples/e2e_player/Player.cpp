//
// Created by Göksu Güvendiren on 28/03/2017.
//

#include <memory>
#include "Player.h"
#include "Video.h"
#include <nanogui/window.h>
#include <nanogui/widget.h>
#include <nanogui/screen.h>

class PlayerImpl
{

};

Player::Player() :
    display_window(1280, 720),
    project_window(1280, 720, nullptr, display_window.get_window())
{
    // GUI STUFF
    using namespace nanogui;

    Screen *screen = new Screen();
    screen->initialize(display_window.get_window(), true);

    Window window = new Window(screen->window(), "name");

    nanogui::Widget *panel = new nanogui::Widget(window);
    window->setPosition(Vector2i(15, 15));
    window->setLayout(new GroupLayout());

    panel->setLayout(new nanogui::BoxLayout(nanogui::BoxLayout::Horizontal, nanogui::BoxLayout::Middle, 0, 20));

    // OTHER STUFF
//    init_player("/Users/goksu/Downloads/office.hdr");
    init_player("/Users/goksu/Desktop/VideoDeghosting/InputVideos/towelHigh.mp4");

    pause();

    float arr[] = {0, 0, 0};
    frame_tex.createFloatBGR(1, 1, arr);

    init_shaders();
    init_quads();
    init_worker();
    init_playback();
}

void Player::init_player(const std::string& path)
{

    // IMAGE
//    auto image2 = cv::imread(path, -1);
//
//    auto size2 = image2.size().width * image2.size().height * 3;
//    auto data2 = std::make_unique<float[]>(size2);
//
//    std::copy((const float*)image2.ptr(), (const float*)image2.ptr() + size2, data2.get());
//    e2e::HDRFrame frame2(std::move(data2), image2.size().width, image2.size().height);
//
//    frames.push(e2e::duplicate(frame2));

    // VIDEO
    Video video(path);

//    for (auto& frame : video.Frames()){
    for (int i = 0; i < 2; i++){
        auto& frame = video.Frames()[i];
        auto size = frame.cols * frame.rows * 3;
        auto data = std::make_unique<float[]>(size);

        std::copy((const float*)frame.data, (const float*)frame.data + size, data.get());
        e2e::HDRFrame hdrframe(std::move(data), frame.cols, frame.rows);

        frames.push(e2e::duplicate(hdrframe));
    }
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
            auto frame = std::move(get_next_frame());

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
            if (display_window.get_key_up(GLFW_KEY_SPACE) || project_window.get_key_up(GLFW_KEY_SPACE))
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
        }
    });
}

e2e::HDRFrame Player::get_next_frame()
{
    auto frame = std::move(Frames().front());
    Frames().pop();

    return frame;
}