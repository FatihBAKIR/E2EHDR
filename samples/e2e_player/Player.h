//
// Created by Göksu Güvendiren on 28/03/2017.
//

#ifndef E2E_PLAYER_PLAYER_H
#define E2E_PLAYER_PLAYER_H

#include <queue>
#include <opencv2/opencv.hpp>
#include <Frame.h>
#include <spsc/spsc_queue.h>

#include <Window.h>
#include <boost/thread.hpp>

#include <texture.h>
#include <glsl_program.h>

class Player
{
    boost::thread work_thread;
    boost::thread vid_thread;

    e2e::Window display_window;
    e2e::Window project_window;
    e2e::spsc_queue<e2e::HDRFrame, e2e::constant_storage<e2e::HDRFrame, 128>> frames;

    e2e::Quad lcd_quad;
    e2e::Quad prj_quad;

    Texture frame_tex;

    Texture ldr_tex;
    Texture resid_tex;

    e2e::GLSLProgram lcd_shader;
    e2e::GLSLProgram prj_shader;

    boost::atomic<bool> is_playing;



    std::set<int> prev_pressed;

    void init_playback();
    void init_quads();
    void init_shaders();
    void init_worker();
    void init_video(const std::string& path);

    void play_loop();
    e2e::HDRFrame get_next_frame();



    void draw_gui();

public:
    Player(const std::string& path);
    ~Player();

    void play();
    void pause();
    void quit();
    void init_player(const std::string& path);

    bool get_playing() const {
        return is_playing.load();
    }

    auto& Frames() { return frames; }
    unsigned int x;
};


#endif //E2E_PLAYER_PLAYER_H
