#include <iostream>
#include "X264Encoder.h"
#include "X264Decoder.hpp"
#include "hdr_encode.hpp"
#include "hdr_decode.hpp"

#include <jpeg/jpeg_decode.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <profiler/profiler.h>
#include <jpeg/jpeg_encode.h>

namespace bio = boost::iostreams;

int main()
{
    init_profiler("hai");

    {
        bio::mapped_file mmap("../catz/cat.jpeg", bio::mapped_file::readonly);
        bio::mapped_file mmap2("../catz/kitten.jpeg", bio::mapped_file::readonly);

        auto begin = (const e2e::byte*) mmap.const_data();
        auto size = mmap.size();

        auto span = gsl::span<const e2e::byte>(begin, begin+size);
        auto kit_span = gsl::span<const e2e::byte>((const e2e::byte*) mmap2.const_data(),
                (const e2e::byte*) mmap2.const_data() + mmap2.size());

        e2e::JpgDecoder decoder(span);
        auto res = decoder.decode(span);
        auto res2 = decoder.decode(kit_span);

        e2e::x264::hdr_encode enc("output.h264", 128, 128);
        for (int i = 0; i < 100; ++i)
        {
            enc.encode(res, res2);
        }
    }

    e2e::x264::hdr_decode dec ("output.h264");

    dec.set_handler([side_count=1] (e2e::x264::decode_result&& res) mutable
    {
        e2e::save_jpeg(res.residual.buffer().data(), 128, 128, "frames/side" + std::to_string(side_count) + ".jpeg");
        e2e::save_jpeg(res.tonemapped.buffer().data(), 128, 128, "frames/frame" + std::to_string(side_count++) + ".jpeg");
    });

    for (int k = 0; k<100; ++k) {
        dec.decode();
    }

    return 0;
}