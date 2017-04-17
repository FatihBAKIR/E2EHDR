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
        class camera;
        class context;

        class stream
        {
            stream_impl* priv;

            friend class camera;
        public:

            stream(const camera& cam, int w, int h, int fps);
            ~stream();

            void start(int exp);
            void stop();

            e2e::spsc_queue<e2e::LDRFrame>& get_frame_queue();
        };

        struct camera_info
        {
            std::string name;
            std::string serial_num;
        };

        class camera
        {
            camera_impl* priv;

            friend class context;
            friend class stream;

            camera(const context&, const camera_info&);

        public:
            camera(const camera&) = delete;
            camera(camera&&);
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
