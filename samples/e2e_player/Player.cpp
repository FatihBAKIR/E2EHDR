//
// Created by Göksu Güvendiren on 28/03/2017.
//

#include <memory>
#include "Player.h"
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>

Player::Player()
{

    auto image2 = cv::imread("/Users/goksu/Downloads/office.hdr", -1);
    auto image1 = cv::imread("/Users/goksu/Desktop/HDRs/belgium.hdr", -1);

    auto size1 = image1.size().width * image1.size().height * 3;
    auto data1 = std::make_unique<float[]>(size1);

    std::copy((const float*)image1.ptr(), (const float*)image1.ptr() + size1, data1.get());
    e2e::HDRFrame frame1(std::move(data1), image1.size().width, image1.size().height);

    auto size2 = image2.size().width * image2.size().height * 3;
    auto data2 = std::make_unique<float[]>(size2);

    std::copy((const float*)image2.ptr(), (const float*)image2.ptr() + size2, data2.get());
    e2e::HDRFrame frame2(std::move(data2), image2.size().width, image2.size().height);

//    for (int i = 0; i < 100; i++) {
    frames.push(e2e::duplicate(frame1));
    frames.push(e2e::duplicate(frame2));
//    }
}


