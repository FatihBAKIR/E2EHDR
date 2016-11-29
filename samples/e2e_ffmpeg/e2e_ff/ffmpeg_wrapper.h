//
// Created by Mehmet Fatih BAKIR on 26/11/2016.
//

#pragma once

#include <stdexcept>
#include <deque>
#include <spsc/spsc_queue.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
namespace e2e
{
namespace ff
{
    using data_ptr = std::unique_ptr<uint8_t[], decltype(&av_free)>;

    class ffmpeg_error : std::runtime_error
    {
    public:
        ffmpeg_error(const char* err) : std::runtime_error(err) {}
        ffmpeg_error(const std::string& err) : std::runtime_error(err) {}
    };

    struct FFmpeg
    {
        FFmpeg()
        {
            static auto _ = ([]{
                av_register_all();
                avformat_network_init();
                return 0;
            })();
        }
    };

    class Camera
    {
    public:
        AVCodec* codec_;
        AVFormatContext* format_ctx_;
        const AVCodecContext* org_codec_ctx_;
        AVCodecContext* codec_ctx_;
        int video_stream_index_;
        AVStream* stream_;

    public:
        struct FrameData
        {
            AVPacket* packet;
            std::chrono::high_resolution_clock::time_point timestamp;
        };

        e2e::spsc_queue<FrameData> packet_queue;
        Camera(const std::string& path);

        /*
         * fills the packet_queue
         * this method is synchronous, call from a separate thread
         */
        void start_capture();
    };

    class Decoder
    {
        AVCodecContext* ctx_;
        AVFrame *pic, *pic_rgb;

        std::deque<data_ptr> pool;
        SwsContext *img_convert_ctx_;
    public:
        Decoder(AVCodecContext* ctx);

        void return_buffer(data_ptr&& buf);

        data_ptr decode_one(AVPacket* p);
    };
}
}