//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#include "jpeg_encode.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace e2e
{
    void save_jpeg(byte* data, int w, int h, const std::string& path)
    {
        cv::Mat m (h, w, CV_8UC3, data);
        cv::cvtColor(m, m, CV_BGR2RGB);
        cv::imwrite(path.c_str(), m);
    }
}