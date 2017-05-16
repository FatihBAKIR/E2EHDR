//
// Created by Mehmet Fatih BAKIR on 21/03/2017.
//

#include <stdexcept>
#include <iostream>

#include "X264Decoder.hpp"

X264Decoder::X264Decoder()
{
    frame = 0;
    buffer_ptr = 0;
    codec_init = false;
    avcodec_register_all();

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);

    if (!codec)
    {
        throw std::runtime_error("failed opening codec");
    }

    codec_context = avcodec_alloc_context3(codec);

    if(codec->capabilities & CODEC_CAP_TRUNCATED) {
        codec_context->flags |= CODEC_FLAG_TRUNCATED | AVFMT_FLAG_KEEP_SIDE_DATA;
    }

    if(avcodec_open2(codec_context, codec, nullptr) < 0) {
        throw std::runtime_error("could not open codec");
    }

    picture = av_frame_alloc();
    pic_rgb = av_frame_alloc();
    parser = av_parser_init(AV_CODEC_ID_H264);

    if(!parser) {
        throw std::runtime_error("cannot create H264 parser");
    }

}

void X264Decoder::got_data(const uint8_t* data, int size)
{
    std::copy(data, data + size, std::back_inserter(buffer));
}

void X264Decoder::decode()
{
    uint8_t* d = nullptr;
    int s = 0;
    auto len = av_parser_parse2(parser, codec_context, &d, &s, &buffer[buffer_ptr],
            buffer.size() - buffer_ptr, 0, 0, AV_NOPTS_VALUE);

    if (s == 0)
    {
        return;
    }

    decode_frame(&buffer[buffer_ptr], s);
    buffer_ptr += len;
}

void X264Decoder::decode_frame(const uint8_t* data, int size)
{
    AVPacket pkt;
    int got_picture = 0;
    int len = 0;

    av_init_packet(&pkt);

    pkt.data = (uint8_t*)data;
    pkt.size = size;

    len = avcodec_decode_video2(codec_context, picture, &got_picture, &pkt);

    if (!codec_init)
    {
        img_convert_ctx_ = sws_getContext(codec_context->width, codec_context->height, codec_context->pix_fmt,
                                      codec_context->width, codec_context->height, AV_PIX_FMT_RGB24,
                                      SWS_BICUBIC, nullptr, nullptr, nullptr);

        int size = avpicture_get_size(AV_PIX_FMT_RGB24, codec_context->width, codec_context->height);
        for (int i = 0; i < 256; ++i)
        {
            pool.emplace_back((uint8_t*)(av_malloc(size)), &av_free);
        }

        codec_init = true;
    }

    if(len < 0) {
        printf("Error while decoding a frame.\n");
    }

    if(got_picture == 0) {
        return;
    }

    ++frame;

    data_ptr buffer = std::move(pool.front());
    pool.pop_front();

    avpicture_fill((AVPicture *) pic_rgb, buffer.get(), AV_PIX_FMT_RGB24, codec_context->width, codec_context->height);

    sws_scale(img_convert_ctx_,
            picture->data, picture->linesize, 0, codec_context->height,
            pic_rgb->data, pic_rgb->linesize); // color conversion

    cb(&pkt, std::move(buffer));
}

X264Decoder::~X264Decoder()
{
    std::cerr << "bye\n";

}
