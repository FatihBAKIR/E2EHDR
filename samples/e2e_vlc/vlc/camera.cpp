//
// Created by Mehmet Fatih BAKIR on 22/11/2016.
//

#include <cstdlib>
#include <iostream>
#include "camera.h"

// VLC prepares to render a video frame.
void *lock(void *data, void **p_pixels)
{
    *p_pixels = new e2e::byte[100 * 1024 * 1024];
    std::cout << "lock" << *p_pixels << '\n';

    return nullptr; // Picture identifier, not needed here.
}

int counter = 0;
// VLC just rendered a video frame.
void unlock(void *data, void *id, void *const *p_pixels)
{
    e2e::byte* test = reinterpret_cast<e2e::byte*>(*p_pixels);

    counter++;

    e2e::LDRFrame f {std::unique_ptr<e2e::byte[]>(test), 1024, 576};

    auto cam = reinterpret_cast<e2e::vlc::Camera*>(data);
    cam->queue.push(std::move(f));

    std::cout << counter << "new frame" << *p_pixels << '\n';
}

e2e::vlc::Camera::Camera(const std::string &path) {
    char const *vlc_argv[] = {
            "--no-audio", // Don't play audio.
            "--no-xlib", // Don't use Xlib.
            //"--verbose=2" // Be much more verbose then normal for debugging purpose

            // Apply a video filter.
            //"--video-filter", "sepia",
            //"--sepia-intensity=200"
    };


    int vlc_argc = std::extent<decltype(vlc_argv)>{};
    setenv("VLC_PLUGIN_PATH", "/Users/fatih/vlc/build/modules", true);

    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if(libvlc == nullptr) {
        throw std::runtime_error("VLC Init failed");
    }

    media = libvlc_media_new_location(libvlc, "rtsp://admin:admin@192.168.0.20/media/video1");
    mp = libvlc_media_player_new_from_media(media);

    auto eman = libvlc_media_player_event_manager(mp);
    eman = libvlc_media_event_manager(media);

    libvlc_event_attach(eman, libvlc_MediaParsedChanged, [](const libvlc_event_t*, void* mp) {

        unsigned w, h;
        libvlc_video_get_size((libvlc_media_player_t*)mp,0,&w,&h);
        std::cout << "parsed" << w << " " << h << '\n';
        libvlc_video_set_format((libvlc_media_player_t*)mp, "RGBA", w, h, w*2);

    }, mp);

    libvlc_media_release(media);

    libvlc_video_set_callbacks(mp, lock, unlock, nullptr, this);
}

void e2e::vlc::Camera::StartPull() {
    libvlc_media_player_play(mp);
}
