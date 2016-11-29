//
// Created by Mehmet Fatih BAKIR on 22/11/2016.
//

#pragma once

#include <vlc/vlc.h>
#include <string>
#include <spsc/spsc_queue.h>
#include <jpeg/jpeg_decode.h>

namespace e2e
{
    namespace vlc
    {
        struct Camera
        {
            spsc_queue<LDRFrame> queue;
            libvlc_instance_t* libvlc;
            libvlc_media_t* media;
            libvlc_media_player_t* mp;

            Camera(const std::string& path);

            void StartPull();
        };
    }
}

