#include <stdexcept>
#include <OpenGL/OpenGL.h>
#include <iostream>
#include "YUV420P.h"

YUV420P::YUV420P()
{
    memset(linesize, 0, sizeof(linesize));
    memset(width, 0, sizeof(width));
    memset(height, 0, sizeof(height));
}

YUV420P::~YUV420P() {
}

//bool YUV420P::setup(VideoBuffer* vb) {
//bool YUV420P::setup(AVDecoderFrame* f) {
bool YUV420P::setup(AVFrame* ff) {

    if(!ff) {
        throw std::runtime_error("Invalid AVFrame");
    }

    // setup width/height/linesizes
    width[0] = ff->width;        // Y
    width[1] = ff->width * 0.5;  // U
    width[2] = ff->width * 0.5;  // V

    height[0] = ff->height;        // Y
    height[1] = ff->height * 0.5;  // U
    height[2] = ff->height * 0.5;  // V

    linesize[0] = ff->linesize[0]; // Y
    linesize[1] = ff->linesize[1]; // U
    linesize[2] = ff->linesize[2]; // V

    // validate
    if(!width[0] || !width[1] || !width[2]) {
        throw std::runtime_error("Cannot validate width");
        return false;
    }
    if(!height[0] || !height[1] || !height[2]) {
        throw std::runtime_error("Cannot validate height");
        return false;
    }
    if(!linesize[0] || !linesize[1] || !linesize[2]) {
        throw std::runtime_error("Cannot validate linesize");
        return false;
    }

    // Setup video textures
    if(!vt_y.setup(width[0], height[0], GL_R8, GL_RED, GL_UNSIGNED_BYTE, linesize[0])) {
        throw std::runtime_error("Cannot setup the VideoTexture for the Y plane");
        return false;
    }

    if(!vt_u.setup(width[1], height[1], GL_R8, GL_RED, GL_UNSIGNED_BYTE, linesize[1])) {
        throw std::runtime_error("Cannot setup the VideoTexture for the U plane");
        return false;
    }

    if(!vt_v.setup(width[2], height[2], GL_R8, GL_RED, GL_UNSIGNED_BYTE, linesize[2])) {
        throw std::runtime_error("Cannot setup the VideoTexture for the V plane");
        return false;
    }

    print();

    return true;
}


bool YUV420P::updateTextures(AVFrame* ff)
{
    if(!ff) {
        throw std::runtime_error("Invalid AVFrame");
    }

    if(!ff->width || !ff->height) {
        throw std::runtime_error("Invalid width / height for the given AVDecoderFrame: " + std::to_string(ff->width) + " x " + std::to_string(ff->height));
    }

    vt_y.setPixels((char*)ff->data[0]);
    vt_u.setPixels((char*)ff->data[1]);
    vt_v.setPixels((char*)ff->data[2]);
    return true;
}


void YUV420P::print()
{
    std::cout << "yuv, plane 0:  width: " << width[0] << ", height: " << height[0] << ", stride: " << linesize[0];
    std::cout << "yuv, plane 0:  width: " << width[1] << ", height: " << height[1] << ", stride: " << linesize[1];
    std::cout << "yuv, plane 0:  width: " << width[2] << ", height: " << height[2] << ", stride: " << linesize[2];
}