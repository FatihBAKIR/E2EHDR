#pragma once

#include <stdexcept>
#include <gphoto2/gphoto2.h>
#include <vector>
#include "../util.h"
#include "../Frame.h"

namespace e2e
{
namespace gp
{
    using CameraInfo = std::pair<std::string, std::string>;

    class GPhoto
    {
        GPContext* ctx_;
        friend class Camera;

    public:
        GPhoto();
        std::vector<CameraInfo> ListCameras();
    };

    void return_cf(::CameraFile* ptr);
    LDRFrame decode(::CameraFile* ptr);

    class CameraFile
    {
        ::CameraFile* internal_;

    public:
        CameraFile(::CameraFile* ptr) : internal_(ptr) {}
        CameraFile(const CameraFile&) = delete;
        CameraFile(CameraFile&& rhs) : internal_(rhs.internal_) {
            rhs.internal_ = nullptr;
        }

        operator gsl::span<const byte>();

        ~CameraFile()
        {
            if (internal_)
                return_cf(internal_);
        }
    };

    class Camera
    {
        ::Camera* ptr_;
        GPhoto& gp_;

    public:
        Camera(const CameraInfo& info, GPhoto& gp);

        CameraFile liveview_frame();

        ~Camera();
    };
}
}
