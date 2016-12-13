#include <iostream>
#include <spdlog/spdlog.h>
#include "Application.h"
#include <profiler/profiler.h>

int main(int argc, char** argv) {
    init_profiler("Main Thread");
    spdlog::stdout_color_mt("console");

    Application app({ "/Users/fatih/cameras/camera_210.json", "/Users/fatih/cameras/camera_110.json" });
    app.Run();
    return 0;
}