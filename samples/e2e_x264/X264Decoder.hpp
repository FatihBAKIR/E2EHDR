//
// Created by Mehmet Fatih BAKIR on 21/03/2017.
//

#ifndef MPEGHDR_X264DECODER_HPP
#define MPEGHDR_X264DECODER_HPP

#include <util.h>
#include <vector>
#include <deque>
#include <functional>
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
class X264Decoder
{
public:
    using data_ptr = std::unique_ptr<uint8_t[], decltype(&av_free)>;
private:
    AVCodec* codec;
    AVCodecContext* codec_context;
    AVCodecParserContext* parser;
    AVFrame* picture;
    AVFrame* pic_rgb;
    int frame;
    int buffer_ptr;

    SwsContext *img_convert_ctx_;
    std::vector<uint8_t> buffer;

    std::function<void(AVPacket*, data_ptr)> cb;
    std::deque<data_ptr> pool;

    bool codec_init;

    void decode_frame(const uint8_t* data, int size);

public:

    X264Decoder();
    ~X264Decoder();

    void set_frame_cb(const std::function<void(AVPacket*, data_ptr)>& c)
    {
        cb = c;
    }

    void return_buffer(data_ptr&& b)
    {
        pool.emplace_back(std::move(b));
    }

    void got_data(const uint8_t* data, int size);
    void decode();
};

#endif //MPEGHDR_X264DECODER_HPP
