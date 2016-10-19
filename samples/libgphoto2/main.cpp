#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "gphoto_wrapper.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int main() {
    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();
    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    cv::namedWindow("hai");

    e2e::gp::Camera c (cams[0], gp);

    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
    int frames = 0;

    while (frames < 60)
    {
        auto frame = c.LiveviewFrame();
        //cv::Mat img(cv::Size(frame.width(), frame.height()), CV_8UC3, frame.buffer().data());
        //cv::cvtColor(img, img, CV_RGB2BGR);
        //cv::imshow("hai", img);
        //cv::waitKey(1);
        //std::this_thread::sleep_for(std::chrono::milliseconds(5));
        frames++;
    }

    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

    auto dur = end - begin;
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    std::cout << 1000 / (total_ms / frames) << '\n';

    return 0;
}