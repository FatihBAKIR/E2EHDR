//
// Created by Mehmet Fatih BAKIR on 28/03/2017.
//

#include "hdr_encode.hpp"
#include <boost/filesystem.hpp>

namespace e2e
{
namespace x264
{

    hdr_encode::hdr_encode(const std::string& output, int w, int h)
    {
        tm_encoder.in_width = w;
        tm_encoder.in_height = h;
        tm_encoder.out_width = w;
        tm_encoder.out_height = h;
        tm_encoder.fps = 24;

        re_encoder.in_width = w;
        re_encoder.in_height = h;
        re_encoder.out_width = w;
        re_encoder.out_height = h;
        re_encoder.fps = 24;

        tm_encoder.in_pixel_format = AV_PIX_FMT_RGB24;
        tm_encoder.out_pixel_format = AV_PIX_FMT_YUV420P;

        re_encoder.in_pixel_format = AV_PIX_FMT_RGB24;
        re_encoder.out_pixel_format = AV_PIX_FMT_YUV420P;

        tm_encoder.open(output, [](auto, auto) { });

        boost::filesystem::path temp = boost::filesystem::unique_path();
        const std::string tempstr    = temp.native();  // optional
        re_encoder.open(tempstr, [&](uint8_t* data, int size) {
            std::copy(data, data + size, std::back_inserter(re_buffer));
        });
    }

    void hdr_encode::encode(const e2e::LDRFrame& tonemapped, const e2e::LDRFrame& residual)
    {
        re_encoder.encode((char*) residual.buffer().data(), std::array<uint8_t, 0>{},
        [&](uint8_t* side_data, int side_size)
        {
            gsl::span<const uint8_t> side_ = {side_data, side_size};
            if (re_buffer.size())
            {
                std::copy(side_data, side_data + side_size, std::back_inserter(re_buffer));
                side_ = re_buffer;
            }
            tm_encoder.encode((char*) tonemapped.buffer().data(), side_,
            [&](uint8_t* data, int size) {});
            re_buffer.clear();
        });
    }

    void hdr_encode::encode(gsl::span<uint8_t> tonemapped, gsl::span<uint8_t> residual)
    {
        re_encoder.encode((char*) residual.data(), std::array<uint8_t, 0>{},
        [&](uint8_t* side_data, int side_size)
        {
            gsl::span<const uint8_t> side_ = {side_data, side_size};
            if (re_buffer.size())
            {
                std::copy(side_data, side_data + side_size, std::back_inserter(re_buffer));
                side_ = re_buffer;
            }
            tm_encoder.encode((char*) tonemapped.data(), side_,
            [&](uint8_t* data, int size) {});
            re_buffer.clear();
        });

    }

}
}

