//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//

#pragma once

#include <memory>
#include <gsl/gsl>

namespace e2e
{
    using byte = unsigned char;

    template <typename ChannelType, int ChannelNum>
    class Frame
    {
        std::unique_ptr<ChannelType[]> buffer_;
        int w_;
        int h_;
    public:

        constexpr Frame(std::unique_ptr<ChannelType[]> buffer, int w, int h) : buffer_(std::move(buffer)), w_(w), h_(h) {}

        auto width() const { return w_; }
        auto height() const { return h_; }
        gsl::span<ChannelType> buffer() const { return { buffer_.get(), w_ * h_ * ChannelNum }; }
        auto u_ptr() { return std::move(buffer_); }
    };

    using LDRFrame = Frame<byte, 3>;

    struct capture_error : public std::runtime_error
    {
        capture_error(const char* str) : std::runtime_error(str) {}
    };

    void init_jpg_pool(int w, int h);
    void return_buffer(LDRFrame);
    LDRFrame decode_jpeg(gsl::span<const byte> data);
}
