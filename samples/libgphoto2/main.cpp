#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "gphoto_wrapper.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>
#include <tinyformat.h>
#include <boost/thread.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <OpenGL/gl.h>

int main() {
    e2e::gp::GPhoto gp;

    auto cams = gp.ListCameras();
    for (auto&& c : cams)
    {
        std::cout << c.first << ' ' << c.second << '\n';
    }

    e2e::gp::Camera c (cams[0], gp);

    volatile bool run = true;

    std::queue<CameraFile*> jpgQueue;
    std::queue<e2e::LDRFrame> frameQueue;
    boost::thread framer([&]{
        int frames = 0;
        jpgQueue.push(c.LiveviewFrame());
        auto begin = std::chrono::system_clock::now();
        while (run)
        {
            jpgQueue.push(c.LiveviewFrame());
            frames++;
        }
        auto end = std::chrono::system_clock::now();
        tfm::printfln("Grab FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));
    });

    boost::thread decoder([&]{
        int frames = 0;

        auto begin = std::chrono::system_clock::now();
        while (run)
        {
            if (jpgQueue.empty()) continue;

            auto file = jpgQueue.front();
            jpgQueue.pop();

            frameQueue.push(e2e::gp::decode(file));

            e2e::gp::return_cf(file);
            frames++;
        }
        auto end = std::chrono::system_clock::now();
        tfm::printfln("Decode FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));
    });

    boost::thread displayer([&]{
        std::cin.get();
        run = false;
    });

    sf::RenderWindow App(sf::VideoMode(800, 600), "SFML window");
    App.setFramerateLimit(60);

    boost::this_thread::sleep_for(boost::chrono::milliseconds(2500));

    int frames = 0;
    sf::Texture texture;
    sf::Sprite sprite;
    sprite.setTexture(texture);
    auto begin = std::chrono::system_clock::now();

    while (App.isOpen() && run) {
        sf::Event Event;
        while (App.pollEvent(Event)) {
            if (Event.type == sf::Event::Closed)
            {
                run = false;
            }
        }

        if (frameQueue.empty()) continue;
        auto& frame = frameQueue.front();

        GLint textureBinding;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
        sf::Texture::bind(&texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width(), frame.height(), GL_RGB, GL_UNSIGNED_BYTE, frame.buffer().data());
        glBindTexture(GL_TEXTURE_2D, textureBinding);

        App.draw(sprite);

        frames++;
        frameQueue.pop();
        App.display();
    }

    auto end = std::chrono::system_clock::now();
    tfm::printfln("Display FPS: %lld", 1000 / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / frames));

    framer.join();
    decoder.join();
    displayer.join();

    return 0;
}