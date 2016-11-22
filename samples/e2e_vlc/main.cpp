#include <iostream>

#include "vlc/camera.h"

int main(int argc, char** argv) {
    e2e::vlc::Camera c("/Users/fatih/Downloads/herb.mp4");

    c.StartPull();
    while(true)
    {
        if(c.queue.empty())
        {
            continue;
        }

        auto frame = std::move(c.queue.front());
        c.queue.pop();

        std::cout << "hai\n";
    }
    return 0;
}