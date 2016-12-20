
#include <iostream>

#include "camera_control.h"

int main()
{
    camera_control control;
    control.set_exposure(1/90000.0f);

    std::cout << control.exp_code() << std::endl;


    return 0;
}
