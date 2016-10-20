#pragma once

#include <stdexcept>
#include <gphoto2/gphoto2.h>
#include <vector>
#include "util.h"

namespace e2e
{
namespace gp
{
    using CameraInfo = std::pair<std::string, std::string>;
    //using JpegContent = std::tuple<

    class GPhoto
    {
        GPContext* ctx_;
        friend class Camera;

    public:
        GPhoto();
        std::vector<CameraInfo> ListCameras();
    };

    void return_cf(CameraFile* ptr);
    LDRFrame decode(CameraFile* ptr);

    class Camera
    {
        ::Camera* ptr_;
        GPhoto& gp_;

    public:
        Camera(const CameraInfo& info, GPhoto& gp);

        CameraFile* LiveviewFrame();

        ~Camera();
    };
}
}
