#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "gphoto_wrapper.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>
#include <tinyformat.h>

int main() {
    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();
    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);

    bool run = true;
    std::condition_variable cv;
    std::queue<e2e::LDRFrame> frameQueue;
    std::thread framer([&]{
        int frames = 0;
        c.LiveviewFrame();
        auto begin = std::chrono::system_clock::now();
        while (run)
        {
            frameQueue.push(c.LiveviewFrame());
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            frames++;
        }
        auto end = std::chrono::system_clock::now();
        tfm::printf("Grab FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));
    });

    std::thread displayer([&]{
        std::cin.get();
        run = false;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    cv::namedWindow("hai");

    int frames = 0;
    auto begin = std::chrono::system_clock::now();
    while (run)
    {
        auto& frame = frameQueue.front();
        cv::Mat img(cv::Size(frame.width(), frame.height()), CV_8UC3, frame.buffer().data());
        cv::cvtColor(img, img, CV_RGB2BGR);
        cv::imshow("hai", img);
        cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        frames++;
        frameQueue.pop();
    }

    auto end = std::chrono::system_clock::now();
    tfm::printf("Display FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));

    framer.join();
    displayer.join();

    return 0;
}