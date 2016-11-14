#pragma once

#include <gphoto2/gphoto2.h>
#include <vector>
#include <boost/circular_buffer.hpp>
#include <util.h>
#include <Frame.h>

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

    class Camera;
    class CameraFile
    {
        Camera& cam;
        ::CameraFile* internal_;
        std::chrono::high_resolution_clock::time_point time;

    public:
        CameraFile(Camera& cam, ::CameraFile* ptr) : cam(cam), internal_(ptr)
        {
            time = std::chrono::high_resolution_clock::now();
        }
        CameraFile(const CameraFile&) = delete;
        CameraFile(CameraFile&& rhs) : cam(rhs.cam), internal_(rhs.internal_), time(rhs.time) {
            rhs.internal_ = nullptr;
        }

        auto get_time() const
        {
            return time;
        }

        operator gsl::span<const byte>();

        ~CameraFile();
    };

    class Camera
    {
        ::Camera* ptr_;
        GPhoto& gp_;
        boost::circular_buffer<::CameraFile*> pool;

    public:
        Camera(const CameraInfo& info, GPhoto& gp);

        CameraFile liveview_frame();
        void return_cf(::CameraFile* f);

        ~Camera();
    };
}
}
