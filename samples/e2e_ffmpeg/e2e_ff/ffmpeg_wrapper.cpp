//
// Created by Mehmet Fatih BAKIR on 26/11/2016.
//

#include <iostream>
#include "ffmpeg_wrapper.h"

using namespace e2e::ff;

e2e::ff::Camera::Camera(const std::string &path) {
    static FFmpeg f;

    format_ctx_ = avformat_alloc_context();

    if(avformat_open_input(&format_ctx_, path.c_str(), nullptr, nullptr) != 0){
        throw ffmpeg_error("Could not open input!");
    }

    if(avformat_find_stream_info(format_ctx_, nullptr) < 0){
        throw ffmpeg_error("Stream info unavailable!");
    }

    auto find_stream = [](auto ctx, auto stream_type = AVMEDIA_TYPE_VIDEO)
    {
        /*
         * Loop through all streams to find the video stream
         */

        for(int i =0;i<ctx->nb_streams;i++)
        {
            if(ctx->streams[i]->codec->codec_type == stream_type)
            {
                return i;
            }
        }
        return -1;
    };

    video_stream_index_ = find_stream(format_ctx_, AVMEDIA_TYPE_VIDEO);
    if (video_stream_index_ < 0)
    {
        throw ffmpeg_error("Cannot find video stream!");
    }

    org_codec_ctx_ = format_ctx_->streams[video_stream_index_]->codec;

    codec_ = avcodec_find_decoder(org_codec_ctx_->codec_id);
    if (!codec_)
    {
        throw ffmpeg_error("codec unavailable!");
    }

    codec_ctx_ = avcodec_alloc_context3(codec_);
    avcodec_get_context_defaults3(codec_ctx_, codec_);
    avcodec_copy_context(codec_ctx_, org_codec_ctx_); // copy org to codec_ctx

    if (avcodec_open2(codec_ctx_, codec_, nullptr) < 0)
    {
        throw ffmpeg_error("avcodec_open2 failed!");
    }

    av_read_play(format_ctx_); //start RTSP

    stream_ = format_ctx_->streams[video_stream_index_];

}

Decoder::Decoder(AVCodecContext *ctx)
        : ctx_{ctx},
          pic{av_frame_alloc()},
          pic_rgb{av_frame_alloc()}
{
    if (!pic || !pic_rgb)
    {
        throw ffmpeg_error("couldn't allocate frames");
    }

    int size = avpicture_get_size(AV_PIX_FMT_RGB24, ctx->width, ctx->height);

    for (int i = 0; i < 256; ++i)
    {
        pool.emplace_back((uint8_t*)(av_malloc(size)), &av_free);
    }

    img_convert_ctx_ = sws_getContext(ctx->width, ctx->height, ctx->pix_fmt,
                                      ctx->width, ctx->height, AV_PIX_FMT_RGB24,
                                      SWS_BICUBIC, nullptr, nullptr, nullptr);

}

void e2e::ff::Decoder::return_buffer(e2e::ff::data_ptr &&buf) {
    pool.emplace_back(std::move(buf));
}

e2e::ff::data_ptr e2e::ff::Decoder::decode_one(AVPacket *p) {
    int is_frame_done;
    int result = avcodec_decode_video2(ctx_, pic, &is_frame_done, p);

    if (is_frame_done)
    {
        data_ptr buffer = std::move(pool.front());
        pool.pop_front();

        avpicture_fill((AVPicture *) pic_rgb, buffer.get(), AV_PIX_FMT_RGB24, ctx_->width, ctx_->height);

        sws_scale(img_convert_ctx_,
                  pic->data, pic->linesize, 0, ctx_->height,
                  pic_rgb->data, pic_rgb->linesize); // color conversion

        return std::move(buffer);
    }

    return {nullptr, &av_free};
}

void Camera::start_capture()
{
    AVPacket* packet = av_packet_alloc();
    while(av_read_frame(format_ctx_, packet) >= 0)
    {
        auto timestamp = std::chrono::high_resolution_clock::now();
        packet_queue.push({packet, timestamp});
        packet = av_packet_alloc();
    }
}


