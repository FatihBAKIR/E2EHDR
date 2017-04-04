#pragma once

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <glad/glad.h>

namespace e2e
{
    class PboDownloader
    {
    public:
        PboDownloader();
        ~PboDownloader();
        int init(GLenum fmt, int w, int h, int num);
        void download();

    public:
        GLenum fmt;
        GLuint* pbos;
        uint64_t num_pbos;
        uint64_t dx;
        uint64_t num_downloads;
        int width;
        int height;
        int nbytes; /* number of bytes in the pbo buffer. */
        unsigned char* pixels; /* the downloaded pixels. */
    };
}