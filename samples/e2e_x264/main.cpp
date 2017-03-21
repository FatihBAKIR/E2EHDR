#include <iostream>
#include "X264Encoder.h"
#include "X264Decoder.hpp"

#include <jpeg/jpeg_decode.h>
#include <jpeg/jpeg_encode.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <profiler/profiler.h>

namespace bio = boost::iostreams;

int main()
{
    init_profiler("hai");
    {
        X264Encoder encoder;
        encoder.in_width = 128;
        encoder.in_height = 128;
        encoder.out_width = 128;
        encoder.out_height = 128;
        encoder.fps = 24;

        encoder.in_pixel_format = AV_PIX_FMT_RGB24;
        encoder.out_pixel_format = AV_PIX_FMT_YUV420P;

        encoder.open("hmmm.h264", [](auto, auto) { });

        X264Encoder side;
        side.in_width = 128;
        side.in_height = 128;
        side.out_width = 128;
        side.out_height = 128;
        side.fps = 24;

        side.in_pixel_format = AV_PIX_FMT_RGB24;
        side.out_pixel_format = AV_PIX_FMT_YUV420P;

        std::vector<uint8_t> side_header;
        side.open("side.h264", [&](uint8_t* data, int size) {
            std::cerr << "header size: " << size << " bytes\n";
            std::copy(data, data + size, std::back_inserter(side_header));
        });

        bio::mapped_file mmap("../catz/cat.jpeg", bio::mapped_file::readonly);

        auto begin = (const e2e::byte*) mmap.const_data();
        auto size = mmap.size();

        auto span = gsl::span<const e2e::byte>(begin, begin+size);

        e2e::JpgDecoder decoder(span);
        auto res = decoder.decode(span);

        int total_bytes = 0;
        for (int j = 0; j<100; ++j) {
            side.encode((char*) res.buffer().data(), std::array<uint8_t, 0>{},
            [&](uint8_t* side_data, int side_size)
            {
                gsl::span<const uint8_t> side_ = {side_data, side_size};
                if (side_header.size())
                {
                    std::copy(side_data, side_data + side_size, std::back_inserter(side_header));
                    side_ = side_header;
                }
                encoder.encode((char*) res.buffer().data(), side_,
                [&](uint8_t* data, int size) {

                    std::cerr << "outputting " << size << " bytes\n";
                    total_bytes += size;
                });
                side_header.clear();
            });

        }

        std::cerr << "total bytes: " << total_bytes << '\n';

        encoder.close();
    }

    bio::mapped_file vid("hmmm.h264", bio::mapped_file::readonly);

    X264Decoder dec;
    X264Decoder side_sei;

    auto extract_sei = [](uint8_t* i) {
        for (; memcmp(i, "###musta", 8); i++);
        i += 8;
        auto begin = i;

        for (; memcmp(i, "fatih###", 8); i++);
        return std::make_pair(begin, i);
    };

    dec.set_frame_cb([&](AVPacket* pk, X264Decoder::data_ptr img) {
        e2e::save_jpeg(img.get(), 128, 128, "test.jpeg");
        dec.return_buffer(std::move(img));

        auto sei = extract_sei(pk->data);

        for(auto i = sei.first; i < sei.second; ++i)
        {
            if (!memcmp(i, "#MU#", 4))
            {
                constexpr char data[] = { 0, 0, 0, 1 };
                memcpy(i, data, 4);
            }
            else if (!memcmp(i, "#M#", 3))
            {
                constexpr char data1[] = { 0, 0, 1 };
                memcpy(i, data1, 3);
            }
            else if (!memcmp(i, "#F#", 3))
            {
                constexpr char data2[] = { 0, 0, 2 };
                memcpy(i, data2, 3);
            }
            else if (!memcmp(i, "#G#", 3))
            {
                constexpr char data3[] = { 0, 0, 3 };
                memcpy(i, data3, 3);
            }
        }

        side_sei.got_data(sei.first, std::distance(sei.first, sei.second));
        side_sei.decode();
    });

    side_sei.set_frame_cb([&](AVPacket* pk, X264Decoder::data_ptr img)
    {
        e2e::save_jpeg(img.get(), 128, 128, "side.jpeg");
        side_sei.return_buffer(std::move(img));
    });

    dec.got_data((const uint8_t*) vid.const_data(), vid.size());
    for (int k = 0; k<100; ++k) {
        dec.decode();
    }

    return 0;
}