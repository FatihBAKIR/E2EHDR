//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//

#include <iostream>
#include <fstream>
#include <cstddef>
#include <jpeglib.h>
#include <util.h>
#include <profiler/profiler.h>
#include <Frame.h>
#include <jpeg/jpeg_decode.h>
#include <memory>

#include <boost/circular_buffer.hpp>

auto read_jpeg_header(gsl::span<const e2e::byte> img)
{
    scope_profile();

    struct jpeg_decompress_struct cinfo;
    jpeg_create_decompress(&cinfo);

    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_mem_src(&cinfo, const_cast<e2e::byte*>(img.data()), img.length());
    jpeg_read_header(&cinfo, TRUE);

    auto width = cinfo.image_width;
    auto height = cinfo.image_height;

    jpeg_destroy_decompress(&cinfo);

    return std::make_tuple(width, height);
}

void read_jpeg_file(gsl::span<const e2e::byte> file, unsigned char* memory)
{
    scope_profile();

    struct jpeg_decompress_struct cinfo;

    JSAMPARRAY buffer;
    int row_stride;

    jpeg_create_decompress(&cinfo);
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_mem_src(&cinfo, const_cast<e2e::byte*>(file.data()), file.length());

    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    {
        named_profile("Buffer Allocation");
        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        auto to = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        std::copy(buffer[0], buffer[0] + row_stride, memory + row_stride * to);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

namespace e2e {
    JpgDecoder::JpgDecoder(gsl::span<const byte> initial_frame) {
        std::tie(width, height) = read_jpeg_header(initial_frame);

        for (int i = 0; i < memory_pool.capacity(); ++i)
        {
            memory_pool.push_back(std::make_unique<e2e::byte []>(width * height * 3));
        }
    }

    LDRFrame JpgDecoder::decode(gsl::span<const byte> data)
    {
        auto mem = std::move(memory_pool.front());
        memory_pool.pop_front();
        read_jpeg_file(data, mem.get());
        return {std::move(mem), width, height};
    }

    void JpgDecoder::return_buffer(LDRFrame frame)
    {
        memory_pool.push_back(frame.u_ptr());
    }
}