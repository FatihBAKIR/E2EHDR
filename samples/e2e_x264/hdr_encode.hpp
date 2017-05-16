//
// Created by Mehmet Fatih BAKIR on 28/03/2017.
//

#ifndef MPEGHDR_HDR_ENCODE_HPP
#define MPEGHDR_HDR_ENCODE_HPP

#include <string>
#include <Frame.h>
#include <vector>
#include "X264Encoder.h"

namespace e2e
{
namespace x264
{
    class hdr_encode
    {
        X264Encoder tm_encoder;
        X264Encoder re_encoder;

        std::vector<uint8_t> re_buffer;
    public:
        hdr_encode(const std::string& output, int w, int h);
        void encode(const e2e::LDRFrame& tonemapped, const e2e::LDRFrame& residual);
        void encode(gsl::span<uint8_t> tonemapped, gsl::span<uint8_t> residual, int w, int h);
    };
}
}

#endif //MPEGHDR_HDR_ENCODE_HPP
