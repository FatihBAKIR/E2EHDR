//
// Created by Göksu Güvendiren on 04/04/2017.
//

#include "Video.h"

Video::Video(const string &name) : _name(name)
{
    auto input = cv::VideoCapture();
    input.open(_name);

    _numFrames = (int) input.get(CV_CAP_PROP_FRAME_COUNT);

    for (int i = 0; i < _numFrames; i++) {
        Mat frame;
        input.read(frame);
        _frames.push_back(std::move(frame));
    }
}
