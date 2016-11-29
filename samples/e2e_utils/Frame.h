//
// Created by Mehmet Fatih BAKIR on 27/10/2016.
//

#pragma once
#include <gsl/span>
#include <memory>
#include <chrono>
#include "util.h"

namespace e2e
{
template <typename ChannelType, int ChannelNum, typename deleter = std::default_delete<ChannelType[]>>
class Frame
{
    std::unique_ptr<ChannelType[], deleter> buffer_;
    short w_;
    short h_;

    std::chrono::high_resolution_clock::time_point time_;
public:

    constexpr Frame(std::unique_ptr<ChannelType[], deleter> buffer, short w, short h)
            : buffer_(std::move(buffer)), w_(w), h_(h) {}

    auto width() const { return w_; }
    auto height() const { return h_; }
    gsl::span<ChannelType> buffer() const { return { buffer_.get(), w_ * h_ * ChannelNum }; }
    auto u_ptr() { return std::move(buffer_); }
    void set_time(std::chrono::high_resolution_clock::time_point time) { time_ = time; }
    auto get_time() const { return time_; }
};

using LDRFrame = Frame<byte, 3>;
}