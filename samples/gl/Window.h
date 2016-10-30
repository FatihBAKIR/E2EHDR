//
// Created by fatih on 29.10.2016.
//

#ifndef GL_WINDOW_H
#define GL_WINDOW_H
struct GLFWwindow;

namespace e2e
{
    class Quad;

    class Window
    {
        int w, h;
        GLFWwindow* window;

    public:
        Window(int width, int height);
        void Loop(const Quad&);
        bool ShouldClose() const;
    };
}

#endif //GL_WINDOW_H
