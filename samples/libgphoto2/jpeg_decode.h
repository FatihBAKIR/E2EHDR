//
// Created by Mehmet Fatih BAKIR on 27/10/2016.
//

#ifndef CAMERA_JPG_DECODE_H_H
#define CAMERA_JPG_DECODE_H_H

#include <boost/circular_buffer.hpp>
#include "util.h"
#include "Frame.h"

namespace e2e
{
    class JpgDecoder
    {
        boost::circular_buffer<std::unique_ptr<e2e::byte[]>> memory_pool {64};
        short width, height;

    public:
        JpgDecoder(gsl::span<const byte> initial_frame);
        LDRFrame decode(gsl::span<const byte> data);
        void return_buffer(LDRFrame frame);
    };

    void init_jpg_pool(int w, int h);
    void return_buffer(LDRFrame);
    LDRFrame decode_jpeg(gsl::span<const byte> data);
}

#endif //CAMERA_JPG_DECODE_H_H
