#include "pbo_downloader.h"
#include <iostream>

namespace e2e
{
    PboDownloader::PboDownloader()
        :fmt(0)
        , pbos(nullptr)
        , num_pbos(0)
        , dx(0)
        , num_downloads(0)
        , width(0)
        , height(0)
        , nbytes(0)
        , pixels(nullptr)
    {}

    PboDownloader::~PboDownloader()
    {
        if (nullptr != pixels)
        {
            delete[] pixels;
            pixels = nullptr;
        }
    }

    int PboDownloader::init(GLenum format, int w, int h, int num)
    {
        if (nullptr != pbos)
        {
            std::cout << "Already initialized. Not necessary to initialize again; or shutdown first." << std::endl;
            return -1;
        }

        if (0 >= num)
        {
            std::cout << "Invalid number of PBOs: " << num << std::endl;
            return -2;
        }

        if (num > 10)
        {
            std::cout << "Asked to create more then 10 buffers; that is probaly a bit too much." << std::endl;
        }

        fmt = format;
        width = w;
        height = h;
        num_pbos = num;

        if (GL_RED == fmt || GL_GREEN == fmt || GL_BLUE == fmt)
        {
            nbytes = width * height;
        }

        else if (GL_RGB == fmt || GL_BGR == fmt)
        {
            nbytes = width * height * 3;
        }

        else if (GL_RGBA == fmt || GL_BGRA == fmt)
        {
            nbytes = width * height * 4;
        }

        else
        {
            std::cout << "Unhandled pixel format, use GL_R, GL_RG, GL_RGB or GL_RGBA." << std::endl;
            return -3;
        }

        if (0 == nbytes)
        {
            std::cout << "Invalid width or height given: " << width << height << std::endl;
            return -4;
        }

        pbos = new GLuint[num];
        if (nullptr == pbos)
        {
            std::cout << "Cannot allocate pbos." << width << height << std::endl;
            return -3;
        }

        pixels = new unsigned char[nbytes];
        if (nullptr == pixels)
        {
            std::cout << "Cannot allocate pixel buffer." << width << height << std::endl;
            return -5;
        }

        glGenBuffers(num, pbos);
        for (int i = 0; i < num; ++i)
        {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[i]);
            glBufferData(GL_PIXEL_PACK_BUFFER, nbytes, nullptr, GL_STREAM_READ);
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        return 0;
    }

    void PboDownloader::download()
    {
        unsigned char* ptr = nullptr;

        if (num_downloads < num_pbos)
        {
            //glMap/Unmap will read from the oldest bound buffer first.
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[dx]);
            //When a GL_PIXEL_PACK_BUFFER is bound, the last 0 is used as offset into the buffer to read into.
            glReadPixels(0, 0, width, height, fmt, GL_UNSIGNED_BYTE, 0);
        }
        else
        {
            //Read from the oldest bound pbo.
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[dx]);

            ptr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            if (nullptr != ptr)
            {
                memcpy(pixels, ptr, nbytes);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }
            else
            {
                std::cout << "Failed to map the buffer" << std::endl;
            }

            //Trigger the next read.
            glReadPixels(0, 0, width, height, fmt, GL_UNSIGNED_BYTE, 0);
        }

        ++dx;
        dx = dx % num_pbos;

        ++num_downloads;
        if (num_downloads == UINT64_MAX)
        {
            num_downloads = num_pbos;
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
}