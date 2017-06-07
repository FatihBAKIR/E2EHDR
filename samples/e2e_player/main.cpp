//FRAMEWORK
#include <glsl_program.h>
#include <quad.h>
#include <Window.h>
#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>
#include "Player.h"
#include <thread>

using namespace e2e;

int main()
{
    std::atomic<Player*> player;
    player.store(nullptr);

    auto t = boost::thread([&]{
        while (player.load() == nullptr);
        player.load()->set_ldr_mode(false);
        player.load()->load_media(image{}, "/home/musti/Downloads/memorial.hdr");
        player.load()->play();
    });

    Player p(1280, 720);
    p.init_player([&]{
        player.store(&p);
    });
}

