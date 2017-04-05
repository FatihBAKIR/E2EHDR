//
// Created by Göksu Güvendiren on 05/04/2017.
//

#pragma once

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

using cv::Mat;
//using cv::VideoCapture;
using cv::VideoWriter;
using std::vector;
using std::string;

class Video
{
    vector<Mat> _frames;
    string _name;
    int _numFrames;

public:
    Video(const string& name);
    Video() : _name("default.avi") {}

    vector<Mat>& Frames() { return _frames; }
    const vector<Mat>& Frames() const { return _frames; }

    int FrameCount() { return _numFrames; }
};


