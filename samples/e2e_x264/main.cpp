#include <iostream>
#include "X264Encoder.h"

#include <jpeg/jpeg_decode.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <profiler/profiler.h>

namespace bio = boost::iostreams;

int main()
{
    init_profiler("hai");

    X264Encoder encoder;
    encoder.in_width  = 128;
    encoder.in_height = 128;
    encoder.out_width  = 128;
    encoder.out_height = 128;
    encoder.fps = 24;

    encoder.in_pixel_format = AV_PIX_FMT_RGB24;
    encoder.out_pixel_format = AV_PIX_FMT_YUV420P;

    encoder.open("hmmm.mp4");

    bio::mapped_file mmap("../catz/cat.jpeg", bio::mapped_file::readonly);

    auto begin = (const e2e::byte*)mmap.const_data();
    auto size = mmap.size();

    auto span = gsl::span<const e2e::byte>(begin, begin + size);

    e2e::JpgDecoder decoder (span);
    auto res = decoder.decode(span);

    int total_bytes = 0;
    for (int j = 0; j<100; ++j) {
        encoder.encode((char*)res.buffer().data(), [&total_bytes](uint8_t* data, int size)
        {
            std::cerr << "outputting " << size << "bytes\n";
            total_bytes += size;
        });
    }

    std::cerr << "total bytes: " << total_bytes << '\n';

    encoder.close();
    return 0;
}