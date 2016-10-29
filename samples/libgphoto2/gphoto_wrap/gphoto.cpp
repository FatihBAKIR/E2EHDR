//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//

#include <iostream>
#include <gphoto2/gphoto2.h>
#include "gphoto_wrapper.h"
#include "jpeglib.h"
#include "../jpeg_decode.h"
#include <cstring>
#include <tinyformat.h>
#include <chrono>
#include <bitset>
#include <queue>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>

namespace
{
    GPContext *create_context() {
        GPContext *context;
        context = gp_context_new();
        gp_context_set_error_func(context, [](auto ctx, auto str, auto data)
        {
            tfm::format(std::cerr, "[CTXERR] %s\n", str);
        }, nullptr);
        gp_context_set_status_func(context, [](auto ctx, auto str, auto data)
        {
            tfm::format(std::cerr, "[CTXSTAT] %s\n", str);
        }, NULL);
        return context;
    }

    int open_camera(Camera **camera, const char *model, const char *port, GPContext *context) {
        static GPPortInfoList		*portinfolist = nullptr;
        static CameraAbilitiesList	*abilities = nullptr;

        int		ret, m, p;
        CameraAbilities	a;
        GPPortInfo	pi;

        ret = gp_camera_new (camera);
        if (ret < GP_OK) return ret;

        if (!abilities) {
            ret = gp_abilities_list_new (&abilities);
            if (ret < GP_OK) return ret;
            ret = gp_abilities_list_load (abilities, context);
            if (ret < GP_OK) return ret;
        }

        m = gp_abilities_list_lookup_model (abilities, model);
        if (m < GP_OK) return ret;
        ret = gp_abilities_list_get_abilities (abilities, m, &a);
        if (ret < GP_OK) return ret;
        ret = gp_camera_set_abilities (*camera, a);
        if (ret < GP_OK) return ret;

        if (!portinfolist) {
            ret = gp_port_info_list_new (&portinfolist);
            if (ret < GP_OK) return ret;
            ret = gp_port_info_list_load (portinfolist);
            if (ret < 0) return ret;
            ret = gp_port_info_list_count (portinfolist);
            if (ret < 0) return ret;
        }

        p = gp_port_info_list_lookup_path (portinfolist, port);
        switch (p) {
            case GP_ERROR_UNKNOWN_PORT:
                fprintf (stderr, "The port you specified "
                                 "('%s') can not be found. Please "
                                 "specify one of the ports found by "
                                 "'gphoto2 --list-ports' and make "
                                 "sure the spelling is correct "
                                 "(i.e. with prefix 'serial:' or 'usb:').",
                         port);
                break;
            default:
                break;
        }
        if (p < GP_OK) return p;

        ret = gp_port_info_list_get_info (portinfolist, p, &pi);
        if (ret < GP_OK) return ret;
        ret = gp_camera_set_port_info (*camera, pi);
        if (ret < GP_OK) return ret;
        return GP_OK;
    }
}

namespace e2e
{
namespace gp
{
    Camera::Camera(const CameraInfo &info, GPhoto &gp) :
            gp_(gp)
    {
        gp_camera_new(&ptr_);
        open_camera(&ptr_, info.first.c_str(), info.second.c_str(), gp_.ctx_);
    }

    Camera::~Camera()
    {
        gp_camera_free(ptr_);
    }

    boost::circular_buffer<::CameraFile*> pool{64};
    int _ = ([] {
        pool.resize(64);
        for (auto& cf : pool)
        {
            gp_file_new(&cf);
        }
        return 0;
    })();

    void return_cf(::CameraFile* ptr)
    {
        pool.push_back(ptr);
    }

    LDRFrame decode(::CameraFile *file)
    {
        Expects(file != nullptr);
        const char* buffer;
        unsigned long size;

        int retval = gp_file_get_data_and_size (file, &buffer, &size);

        if (retval != GP_OK)
        {
            throw capture_error("File get data failed");
        }

        Ensures(buffer != nullptr);
        return e2e::decode_jpeg({reinterpret_cast<const byte*>(buffer), static_cast<long>(size)});
    }

    CameraFile Camera::liveview_frame()
    {
        auto file = pool.front();
        pool.pop_front();

        int retval = gp_camera_capture_preview(ptr_, file, gp_.ctx_);

        if (retval != GP_OK)
        {
            throw capture_error("Preview capture failed");
        }

        return {file};
    }

    GPhoto::GPhoto() : ctx_(create_context()) {}

    std::vector<CameraInfo> GPhoto::ListCameras()
    {
        CameraList *cams;
        gp_list_new(&cams);

        gp_list_reset(cams);
        gp_camera_autodetect(cams, ctx_);

        std::vector<CameraInfo> cameras;

        for (int i = 0; i < gp_list_count(cams); i++)
        {
            const char *name, *value;
            gp_list_get_name(cams, i, &name);
            gp_list_get_value(cams, i, &value);
            cameras.emplace_back(name, value);
        }

        gp_list_free(cams);

        return cameras;
    };

    CameraFile::operator gsl::span<const byte>()
    {
        const char *buffer;
        unsigned long size;

        int retval = gp_file_get_data_and_size(internal_, &buffer, &size);

        if (retval != GP_OK) {
            throw capture_error("File get data failed");
        }

        return {reinterpret_cast<const byte*>(buffer), static_cast<long>(size)};
    }
}
}