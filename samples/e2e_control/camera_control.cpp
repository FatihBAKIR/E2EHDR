//
// Created by Göksu Güvendiren on 20/12/2016.
//

#include <cmath>
#include "camera_control.h"
#include <cpr/cpr.h>
#include <iostream>

camera_control::camera_control()
{
    expValues.push_back({1.0f, 0});
    expValues.push_back({1/2.0f, 1});
    expValues.push_back({1/4.0f, 2});
    expValues.push_back({1/8.0f, 3});
    expValues.push_back({1/15.0f, 4});
    expValues.push_back({1/30.0f, 5});
    expValues.push_back({1/50.0f, 6});
    expValues.push_back({1/60.0f, 7});
    expValues.push_back({1/100.0f, 8});
    expValues.push_back({1/250.0f, 9});
    expValues.push_back({1/500.0f, 10});
    expValues.push_back({1/1000.0f, 11});
    expValues.push_back({1/2000.0f, 12});
    expValues.push_back({1/4000.0f, 13});
    expValues.push_back({1/10000.0f, 14});
}

void camera_control::set_exposure(float exp)
{
    expToCode expValue;
    auto minDifference = 100.0;
    for (auto& value : expValues){
        auto difference = std::fabs(value.exp - exp);
        if (difference < minDifference) {
            minDifference = difference;
            expValue = value;
        }
    }

    _exposure = expValue.exp;
    _code = expValue.code;

    post();
}

void camera_control::post()
{
    auto r = cpr::Post(cpr::Url{"http://192.168.200.20/command/camera.cgi"},
                       cpr::Payload{{"BLComp", "off"},
                                    {"ExpComp", "6"},
                                    {"Agc","off"},
                                    {"AutoSlowShutter", "on"},
                                    {"AutoSlowShutterMinSpeed", std::to_string(exp_code())},
                                    {"AutoShutter", "on"},
                                    {"AutoShutterMaxSpeed", std::to_string(exp_code())},
                                    {"WBMode", "atw"},
                                    {"VideoNoiseReduction", "on"},
                                    {"Gamma", "0"},
                                    {"Brightness", "5"},
                                    {"Saturation", "3"},
                                    {"Sharpness", "3"},
                                    {"Contrast", "3"},
                                    {"reload", "referer"}},
                       cpr::Header{{"Cookie", "snc_cview_auto_play="},
                                   {"Origin", "http://192.168.200.20"},
                                   {"Accept-Encoding", "gzip, deflate"},
                                   {"Accept-Language", "en-US,en;q=0.8,tr;q=0.6"},
                                   {"Upgrade-Insecure-Requests", "1"},
                                   {"Authorization", "Basic YWRtaW46YWRtaW4="},
                                   {"Content-Type", "application/x-www-form-urlencoded"},
                                   {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
                                   {"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.95 Safari/537.36"},
                                   {"Cache-Control", "max-age=0"},
                                   {"Referer", "http://192.168.200.20/en/l4/camera/picture.html"},
                                   {"Connection", "keep-alive"},
                                   {"DNT", "1"}
                       });

    std::cout << r.status_code << std::endl;                  // 200
    r.header["content-type"];
    std::cout << r.text << std::endl;

}