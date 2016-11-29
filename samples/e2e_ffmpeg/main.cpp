#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <spsc/spsc_queue.h>
#include <thread>
#include <future>
#include <deque>

#include "e2e_ff/ffmpeg_wrapper.h"

int main(int argc, char** argv)
{
    Camera c {"rtsp://admin:admin@192.168.0.20/media/video1"};

    e2e::spsc_queue<AVPacket*> q;

    std::thread t (
            [&]{
                c.start_capture([&](AVPacket* packet)
                                {
                                    q.emplace(packet);
                                });
            }
    );

    Decoder d{c.codec_ctx_};

    auto handle_frame = [&](int w, int h, Decoder::data_ptr data)
    {
        std::cout << "frame " << w << ", " << h << '\n';
        d.return_buffer(std::move(data));
    };

    while (true)
    {
        if (!q.empty())
        {
            AVPacket* packet = q.front();
            q.pop();

            if(packet->stream_index == c.video_stream_index_)
            {
                auto ret = d.decode_one(packet);

                if (ret)
                {
                    handle_frame(c.codec_ctx_->width, c.codec_ctx_->height, std::move(ret));
                }
            }

            av_packet_unref(packet);
        }
    }

    av_read_pause(c.format_ctx_);

    return 0;
}