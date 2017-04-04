//
// Created by Göksu Güvendiren on 03/01/2017.
//

#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <string>
#include <gsl/span>

extern "C"
{
#include <x264.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

class X264Encoder
{
public:
    X264Encoder();
    ~X264Encoder();
    bool open(std::string filename, std::function<void(uint8_t*, int)> handler);                              /* open for encoding */
    bool encode(char* pixels, gsl::span<const uint8_t> side, std::function<void(uint8_t* data, int len)> handler);                                    /* encode the given data */
    bool close();                                                 /* close the encoder and file, frees all memory */
private:
    bool validateSettings();                                      /* validates if all params are set correctly, like width,height, etc.. */
    void setParams();                                             /* sets the x264 params */

public:
    /* params the user should set */
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    int fps;                                                       /* e.g. 25, 60, etc.. */
    AVPixelFormat in_pixel_format;
    AVPixelFormat out_pixel_format;

    /* x264 */
    AVPicture pic_raw;                                            /* used for our "raw" input container */
    x264_picture_t pic_in;
    x264_picture_t pic_out;
    x264_param_t params;
    x264_nal_t* nals;
    x264_t* encoder;
    int num_nals;

    /* input / output */
    int pts;
    struct SwsContext* sws;
    FILE* fp;
};
