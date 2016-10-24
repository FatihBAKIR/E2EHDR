//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//


#include <iostream>
#include <fstream>
#include <cstddef>
#include <jpeglib.h>
#include "util.h"
#include "profiler.h"
#include <memory>
#include <tinyformat.h>

#include <boost/circular_buffer.hpp>

boost::circular_buffer<std::unique_ptr<e2e::byte[]>> memory_pool {64};

void e2e::init_jpg_pool(int w, int h)
{
    for (int i = 0; i < (int)memory_pool.capacity(); ++i)
    {
        memory_pool.push_back(std::make_unique<e2e::byte []>(w * h * 3));
    }
}

void e2e::return_buffer(LDRFrame frame)
{
    memory_pool.push_back(frame.u_ptr());
}

auto read_JPEG_file (const e2e::byte* data, unsigned long size)
{
    //profile();
    struct jpeg_decompress_struct cinfo;

    JSAMPARRAY buffer;
    int row_stride;

    jpeg_create_decompress(&cinfo);
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_mem_src(&cinfo, const_cast<e2e::byte*>(data), size);

    jpeg_read_header(&cinfo, TRUE);

    //tfm::printfln("%d, %d", cinfo.image_width, cinfo.image_height);

    auto memory = memory_pool.size() ? std::move(memory_pool.front()) : std::make_unique<unsigned char[]>(cinfo.image_width * cinfo.image_height * 3);
    if (memory_pool.size()) memory_pool.pop_front();

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        auto to = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        std::copy(buffer[0], buffer[0] + row_stride, memory.get() + row_stride * to);
    }

    auto ret = std::make_tuple(std::move(memory), cinfo.image_width, cinfo.image_height);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return ret;
}

e2e::LDRFrame e2e::decode_jpeg(gsl::span<const byte> data)
{
    //return e2e::LDRFrame(nullptr, 0, 0);

    auto tup = read_JPEG_file(data.data(), data.length());
    return e2e::LDRFrame(std::move(std::get<0>(tup)), std::get<1>(tup), std::get<2>(tup));
}
