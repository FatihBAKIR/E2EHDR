//
// Created by Mehmet Fatih BAKIR on 19/12/2016.
//

#pragma once

#include <json.hpp>
#include <e2e_ff/ffmpeg_wrapper.h>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include <Frame.h>
#include <configuration.h>

class camera_struct // per camera data
{
public:
    using FrameT = e2e::Frame<uint8_t, 3, decltype(&av_free)>;

    /// decoded frames are stored in this queue
    e2e::spsc_queue<FrameT, e2e::constant_storage<FrameT, 128>> frame_queue;

    camera_struct(const nlohmann::json& config);

    void start();

    void stop();

    void recycle(FrameT&& f);

    float get_exposure() const;

    const crf& get_response() const;

    undistort get_undistort() const;
    const std::string& get_name() const;
    const std::string& get_ip() const;

    camera_struct(camera_struct&) = delete;
    camera_struct(camera_struct&&) = delete;

    ~camera_struct();
private:

    nlohmann::json      config;
    e2e::ff::Camera     camera;

    boost::thread       pull_thread;

    e2e::ff::Decoder    decoder;
    boost::thread       decode_thread;

    std::string         name;
    std::string         ip;
    boost::optional<FrameT> handle_packet(e2e::ff::Camera::FrameData frame_data);
    mutable boost::optional<crf> response;
};
