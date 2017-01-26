#include <iostream>
#include "X264Encoder.h"

int main()
{


    X264Encoder encoder;
    encoder.in_width  = 128;
    encoder.in_height = 128;
    encoder.out_width  = 128;
    encoder.out_height = 128;

    encoder.in_pixel_format = AV_PIX_FMT_YUV420P;
    encoder.out_pixel_format = AV_PIX_FMT_YUV420P;
    encoder.open("hmmm.bin");



    encoder.close();
    return 0;
}