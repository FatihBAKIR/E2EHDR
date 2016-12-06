//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#ifndef E2E_UTILS_JPEG_ENCODE_H_H
#define E2E_UTILS_JPEG_ENCODE_H_H


#include <util.h>

namespace e2e
{
    void save_jpeg(byte* data, int w, int h, const std::string& path);
}

#endif //E2E_UTILS_JPEG_ENCODE_H_H
