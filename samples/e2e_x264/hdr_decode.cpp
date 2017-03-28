//
// Created by Mehmet Fatih BAKIR on 28/03/2017.
//

#include "hdr_decode.hpp"
#include <Frame.h>

namespace e2e {
    namespace x264 {
        auto extract_sei(uint8_t* i) {
            for (; memcmp(i, "###musta", 8); i++);
            i += 8;
            auto begin = i;

            for (; memcmp(i, "fatih###", 8); i++);
            return std::make_pair(begin, i);
        };

        namespace bio = boost::iostreams;

        hdr_decode::hdr_decode(const std::string& path)
        {
            file = bio::mapped_file(path, bio::mapped_file::readonly);
            tm_decoder.got_data((const uint8_t*) file.const_data(), file.size());

            tm_decoder.set_frame_cb([&](AVPacket* pk, X264Decoder::data_ptr img) {
                auto sei = extract_sei(pk->data);

                for (auto i = sei.first; i<sei.second; ++i) {
                    if (!memcmp(i, "#MU#", 4)) {
                        constexpr char data[] = {0, 0, 0, 1};
                        memcpy(i, data, 4);
                    }
                    else if (!memcmp(i, "#M#", 3)) {
                        constexpr char data1[] = {0, 0, 1};
                        memcpy(i, data1, 3);
                    }
                    else if (!memcmp(i, "#F#", 3)) {
                        constexpr char data2[] = {0, 0, 2};
                        memcpy(i, data2, 3);
                    }
                    else if (!memcmp(i, "#G#", 3)) {
                        constexpr char data3[] = {0, 0, 3};
                        memcpy(i, data3, 3);
                    }
                }

                next.tonemapped = frame_t{std::move(img), 128, 128};

                re_decoder.got_data(sei.first, std::distance(sei.first, sei.second));
                re_decoder.decode();
            });

            re_decoder.set_frame_cb([&](AVPacket* pk, X264Decoder::data_ptr img)
            {
                next.residual = frame_t{std::move(img), 128, 128};
                handler(std::move(next));
            });
        }

        void hdr_decode::decode()
        {
            tm_decoder.decode();
        }

        void hdr_decode::return_result(decode_result&& res)
        {
            tm_decoder.return_buffer(std::move(res.tonemapped.u_ptr()));
            re_decoder.return_buffer(std::move(res.residual.u_ptr()));
        }
    }
}

