//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//

#pragma once

#include <gsl/span>

namespace e2e
{
    template <typename ChannelType, int ChannelNum>
    class Frame
    {
        std::unique_ptr<ChannelType[]> buffer_;
    public:

        
    };

    using LDRFrame = Frame<gsl::byte, 3>;

    struct capture_error : public std::runtime_error
    {
        capture_error(const char* str) : std::runtime_error(str) {}
    };

    LDRFrame decode_jpeg(gsl::span<const gsl::byte> data);
}
