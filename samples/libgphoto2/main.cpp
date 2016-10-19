#include <iostream>
#include <thread>
#include <vector>
#include "gphoto_wrapper.h"

int main() {
    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();
    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);
    c.LiveviewFrame();

    return 0;
}