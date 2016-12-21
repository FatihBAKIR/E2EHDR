//
// Created by Göksu Güvendiren on 20/12/2016.
//

#ifndef E2E_CONTROL_CAMERA_CONTROL_H
#define E2E_CONTROL_CAMERA_CONTROL_H

#include <vector>
#include <string>

class camera_control
{
    float _exposure;
    int _code;
    std::string _ip;

    struct expToCode
    {
        float exp;
        int code;
    };

    std::vector<expToCode> expValues;
    void post();

public:
    camera_control(std::string ip);
    void set_exposure(float exp);
    float exp_value() { return _exposure; };
    int exp_code() { return _code; };

};


#endif //E2E_CONTROL_CAMERA_CONTROL_H
