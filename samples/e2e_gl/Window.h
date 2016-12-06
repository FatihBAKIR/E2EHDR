//
// Created by fatih on 29.10.2016.
//

#pragma once

//CPP
#include <functional>
#include <vector>

struct GLFWwindow;

namespace e2e
{
    class Quad;

    class Window
    {
        int m_width, m_height;
        GLFWwindow* m_window;

    public:
        Window(int width, int height);
        void Loop(const std::vector<std::reference_wrapper<Quad>>& quads);
        bool ShouldClose() const;

		GLFWwindow* get_window() const { return m_window; }
    };
}