//
// Created by Mehmet Fatih BAKIR on 17/04/2017.
//

#include "camera.hpp"
#include <libuvc/libuvc.h>
#include <iostream>
#include <spsc/spsc_queue.h>
#include <Frame.h>

namespace e2e {
    namespace uvc {

        using uvc_frame_ptr = std::unique_ptr<uvc_frame_t, decltype(&uvc_free_frame)>;
        struct context_impl
        {
            uvc_context_t* ctx;
        };

        struct camera_impl
        {
            uvc_device_t* dev;
            uvc_device_handle_t* devh;
            const context* ctx;
        };

        struct stream_impl
        {
            const camera* cam;
            uvc_stream_ctrl_t ctrl;
            e2e::spsc_queue<uvc_frame_ptr, e2e::constant_storage<uvc_frame_ptr, 256>> raw_frames;
            e2e::spsc_queue<e2e::LDRFrame> frames;

            void handle_frame(uvc_frame_t* frame)
            {
                e2e::LDRFrame fr(std::make_unique<uint8_t[]>(frame->width * frame->height * 3),
                        frame->width, frame->height);

                uvc_frame_t f;
                f.data = fr.buffer().data();
                f.data_bytes = fr.buffer().size_bytes();
                f.library_owns_data = 0;

                std::cerr << frame->frame_format << ", " << UVC_FRAME_FORMAT_MJPEG << '\n';
                auto ret = uvc_mjpeg2rgb(frame, &f);
                if (ret < 0)
                {
                    uvc_perror(ret, "mjpeg2rgb");
                }
                else
                {
                    frames.emplace(std::move(fr));
                }
            }
        };

        camera::camera(const context& ctx, const camera_info& info)
        {
            priv = new camera_impl;
            priv->ctx = &ctx;

            auto res = uvc_find_device(priv->ctx->priv->ctx, &priv->dev, 0, 0, info.serial_num.c_str());
            if (res < 0)
            {
                throw "can't create device";
            }

            res = uvc_open(priv->dev, &priv->devh);

            if (res < 0)
            {
                throw "can't open device";
            }

            uvc_set_backlight_compensation(priv->devh, 0);
        }

        camera::~camera()
        {
            uvc_close(priv->devh);
            uvc_unref_device(priv->dev);
            delete priv;
        }

        camera::camera(camera&& rhs)
        {
            priv = std::exchange(rhs.priv, nullptr);
        }

        /*void format_descs()
        {
            auto rs = uvc_get_format_descs(devh);

            while (rs!=nullptr) {
                std::cout << (int) rs->bFormatIndex << '\n';
                auto d = rs->frame_descs;
                while (d!=nullptr) {
                    std::cout << d->wWidth << ", " << d->wHeight << ", " << d->dwDefaultFrameInterval << '\n';
                    d = d->next;
                }
                rs = rs->next;
            }
        }*/

        stream::stream(const camera& cam, int w, int h, int fps)
        {
            priv = new stream_impl;
            priv->cam = &cam;
            auto res = uvc_get_stream_ctrl_format_size(
                cam.priv->devh, &priv->ctrl,
                UVC_FRAME_FORMAT_ANY,
                w, h, fps
            );

            if (res < 0)
            {
                throw "can't create stream";
            }
        }

        void stream::start()
        {
            auto res = uvc_start_streaming(priv->cam->priv->devh, &priv->ctrl, [](uvc_frame_t* f, void* opaque) {
                reinterpret_cast<decltype(this)>(opaque)->priv->handle_frame(f);
            }, this, 0);

            if (res < 0)
            {
                throw "can't start stream";
            }
        }

        void stream::stop()
        {
            uvc_stop_streaming(priv->cam->priv->devh);
        }

        stream::~stream()
        {
            stop();
            delete priv;
        }

        e2e::spsc_queue<e2e::LDRFrame>& stream::get_frame_queue()
        {
            return priv->frames;
        }

        void camera::set_exposure(int exp)
        {
            if (uvc_set_ae_mode(priv->devh, 1) < 0)
            {
                std::cerr << "couldn't set ae mode\n";
            }
            if (uvc_set_exposure_abs(priv->devh, exp) < 0)
            {
                std::cerr << "couldn't set exposure\n";
            }
        }

        int camera::get_exposure() const
        {
            uint32_t exp;
            uvc_get_exposure_abs(priv->devh, &exp, uvc_req_code::UVC_GET_CUR);
            return exp;
        }

        void camera::set_shutter_speed(std::chrono::milliseconds arg)
        {
            long ms = std::chrono::duration_cast<std::chrono::milliseconds>(arg).count();
            int val = ms * 10;
            set_exposure(val);
        }

        std::chrono::milliseconds camera::get_shutter_speed() const
        {
            auto v = get_exposure();
            return std::chrono::milliseconds(v / 10);
        }

        uint16_t camera::get_iso() const
        {
            uint16_t gain;
            uvc_get_gain(priv->devh, &gain, uvc_req_code::UVC_GET_CUR);
            return gain;
        }

        void camera::set_iso(uint16_t iso)
        {
            uvc_set_gain(priv->devh, iso);
        }

        void camera::dump() const
        {
            uint16_t max_iso, min_iso, def_iso;
            uvc_get_gain(priv->devh, &min_iso, uvc_req_code::UVC_GET_MIN);
            uvc_get_gain(priv->devh, &max_iso, uvc_req_code::UVC_GET_MAX);
            uvc_get_gain(priv->devh, &def_iso, uvc_req_code::UVC_GET_DEF);

            std::cout << "(" << min_iso << ", " << max_iso  << ", " << def_iso << ")\n";
        }

        void camera::set_wb_temp(uint16_t temp)
        {
            uvc_set_white_balance_temperature_auto(priv->devh, 0);
            uvc_set_white_balance_temperature(priv->devh, temp);
        }
    }
}

std::vector<e2e::uvc::camera_info> e2e::uvc::context::list_cameras()
{
    std::vector<e2e::uvc::camera_info> cams;
    uvc_device_t** devs;
    uvc_get_device_list(priv->ctx, &devs);

    while (*devs!=nullptr) {
        uvc_device_descriptor_t* d;
        uvc_get_device_descriptor(devs[0], &d);

        if (!d->product)
        {
            cams.push_back({ std::to_string(d->idProduct), std::to_string(d->idVendor) });
        }
        else
        {
            cams.push_back({ d->product, d->serialNumber });
        }

        uvc_free_device_descriptor(d);
        ++devs;
    }
    return cams;
}

e2e::uvc::context::context()
{
    priv = new context_impl;
    uvc_init(&priv->ctx, nullptr);
}

e2e::uvc::context::~context()
{
    uvc_exit(priv->ctx);
    delete priv;
}

e2e::uvc::camera e2e::uvc::context::open_camera(const e2e::uvc::camera_info& inf)
{
    return e2e::uvc::camera(*this, inf);
}
