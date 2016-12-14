//
// Created by Mehmet Fatih BAKIR on 13/12/2016.
//

#ifndef HDR_CAMERA_APPLICATION_H
#define HDR_CAMERA_APPLICATION_H

class ApplicationImpl;

class Application {

    std::unique_ptr<ApplicationImpl> impl;

public:
    Application(const std::vector<std::string>& args);

    void Run();

    ~Application();
};


#endif //HDR_CAMERA_APPLICATION_H
