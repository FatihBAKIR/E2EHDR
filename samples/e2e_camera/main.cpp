#include <iostream>
#include <spdlog/spdlog.h>
#include "Application.h"
#include <profiler/profiler.h>

int main(int argc, char** argv) {
    init_profiler("Main Thread");
    spdlog::stdout_color_mt("console");

    Application app({ "C:\\Users\\Mustafa\\Documents\\E2EHDR.isikmustafa\\cameras\\camera_210.json", 
		"C:\\Users\\Mustafa\\Documents\\E2EHDR.isikmustafa\\cameras\\camera_110.json" });
    app.Run();
    return 0;
}