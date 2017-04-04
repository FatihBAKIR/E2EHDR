//
// Created by Mehmet Fatih BAKIR on 28/03/2017.
//

#pragma once

#include <Frame.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include "X264Decoder.hpp"

namespace e2e
{
namespace x264
{
    using frame_t = e2e::Frame<uint8_t, 3, decltype(&av_free)>;

    struct decode_result
    {
        frame_t tonemapped = {{nullptr, &av_free}, 0, 0};
        frame_t residual = {{nullptr, &av_free}, 0, 0};
    };

    class hdr_decode
    {
        boost::iostreams::mapped_file file;
        X264Decoder tm_decoder;
        X264Decoder re_decoder;

        std::function<void(decode_result&&)> handler;

        decode_result next;

    public:
        hdr_decode(const std::string& path);
        hdr_decode(const hdr_decode&) = delete;
        hdr_decode(hdr_decode&&) = delete;

        void set_handler(std::function<void(decode_result&&)> h)
        {
            handler = h;
        }

        void decode();
        void return_result(decode_result&& res);
    };
}
}

