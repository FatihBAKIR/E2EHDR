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

#include "shared_frame_queue.hpp"
#include <boost/variant.hpp>

struct video {};
struct image {};

class Player
{
    boost::thread work_thread;
    boost::thread vid_thread;
    boost::thread play_thread;

    e2e::Window* display_window;
    e2e::Window* project_window;

    using half_frames_t = std::unique_ptr<e2e::spsc_queue<e2e::HalfFrame, e2e::constant_storage<e2e::HalfFrame, 512>>>;
    using float_frames_t = std::unique_ptr<e2e::spsc_queue<e2e::HDRFrame, e2e::constant_storage<e2e::HDRFrame, 64>>>;
    using frames_t = boost::variant<half_frames_t, float_frames_t>;

    frames_t frames;

    short m_width, m_height;

    e2e::Quad lcd_quad;
    e2e::Quad prj_quad;

    Texture frame_tex;

    Texture ldr_tex;
    Texture resid_tex;

    e2e::GLSLProgram lcd_shader;
    e2e::GLSLProgram prj_shader;

    boost::atomic<bool> is_playing;

    float im_scale_x, im_scale_y;
    boost::atomic<bool> ldr_mode;

    int corner_id = 0;

    std::set<int> prev_pressed;

    e2e::shared_frames_queue mq = {false, 1280, 720};

    void init_playback();
    void init_quads();
    void init_shaders();
    void init_worker();
    void init_video(const std::string& path);
    void init_ipc();

    void play_loop();

    void draw_gui();

    std::pair<short, short> calculate_size(const std::pair<short, short>&) const;

public:
    Player(short w, short h);
    ~Player();

    void load_media(video, const std::string& path, short w, short h);
    void load_media(image, const std::string& path);

    void play();
    void pause();
    void quit();
    void init_player(std::function<void()> pre_play);

    void set_ldr_mode(bool ldr);

    bool get_playing() const {
        return is_playing.load();
    }

    auto& Frames() { return frames; }
    unsigned int x;
};


#endif //E2E_PLAYER_PLAYER_H
