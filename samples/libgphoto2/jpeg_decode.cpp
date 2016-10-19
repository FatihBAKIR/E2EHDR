//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//


#include <iostream>
#include <fstream>
#include <cstddef>
#include <jpeglib.h>
#include "util.h"
#include <memory>

auto read_JPEG_file (const gsl::byte* data, unsigned long size)
{
    struct jpeg_decompress_struct cinfo;

    JSAMPARRAY buffer;
    int row_stride;

    jpeg_create_decompress(&cinfo);
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_mem_src(&cinfo, reinterpret_cast<unsigned char*>(const_cast<gsl::byte*>(data)), size);

    jpeg_read_header(&cinfo, TRUE);
    std::cout << cinfo.image_width << ',' << cinfo.image_height << '\n';

    auto memory = std::unique_ptr<unsigned char[]>{new unsigned char[cinfo.image_width * cinfo.image_height * 3]};

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {

        jpeg_read_scanlines(&cinfo, buffer, 1);
        //put_scanline_someplace(buffer[0], row_stride);

        std::copy(buffer[0], buffer[0] + cinfo.image_width, memory.get() + cinfo.image_width * cinfo.output_scanline);
        std::cout << row_stride << '\n';
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return std::make_tuple(std::move(memory), cinfo.image_width, cinfo.image_height);
}

e2e::LDRFrame e2e::decode_jpeg(gsl::span<const gsl::byte> data)
{
    auto tup = read_JPEG_file(data.data(), data.length());

    return { std::get<0>(tup) };
}