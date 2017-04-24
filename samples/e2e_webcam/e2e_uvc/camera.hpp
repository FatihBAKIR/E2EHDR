//
// Created by Mehmet Fatih BAKIR on 17/04/2017.
//

#ifndef E2E_WEBCAM_CAMERA_HPP
#define E2E_WEBCAM_CAMERA_HPP

#include <string>
#include <vector>
#include <spsc/spsc_queue.h>
#include <Frame.h>

namespace e2e {
    namespace uvc {
        struct context_impl;
        struct camera_impl;
        struct stream_impl;
        struct info_impl;
        class camera;
        class context;

        class stream
        {
            stream_impl* priv;

            friend class camera;
        public:

            stream(const camera& cam, int w, int h, int fps);
            ~stream();

            void start();
            void stop();

            e2e::spsc_queue<e2e::LDRFrame>& get_frame_queue();
        };

        struct camera_info
        {
            void* dev;
            std::string name;
            std::string serial_num;
        };

        class camera
        {
            camera_impl* priv;

            friend class context;
            friend class stream;

            camera(const context&, const camera_info&);

            void set_exposure(int exp);
            int get_exposure() const;
        public:
            camera(const camera&) = delete;
            camera(camera&&);

            void set_shutter_speed(std::chrono::milliseconds);
            std::chrono::milliseconds get_shutter_speed() const;

            void set_auto_wb(bool s);
            void set_wb_temp(uint16_t temp);

            void set_iso(uint16_t iso);
            uint16_t get_iso() const;

            void dump() const;

            ~camera();
        };

        class context
        {
            context_impl* priv;
            friend class camera;
        public:
            context();
            ~context();

            context(const context&) = delete;
            context(context&&) = delete;
            std::vector<camera_info> list_cameras();
            camera open_camera(const camera_info&);
        };


    }
}

#endif //E2E_WEBCAM_CAMERA_HPP
