#include <cpr/cpr.h>
#include <iostream>

int main()
{
    auto r = cpr::Post(cpr::Url{"http://192.168.200.20/command/camera.cgi"},
                       cpr::Payload{{"BLComp", "off"},
                                     {"ExpComp", "6"},
                                     {"Agc","off"},
                                     {"AutoSlowShutter", "on"},
                                     {"AutoSlowShutterMinSpeed", "7"},
                                     {"AutoShutter", "on"},
                                     {"AutoShutterMaxSpeed", "7"},
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

    return 0;
}
