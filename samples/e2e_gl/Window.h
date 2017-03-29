//
// Created by fatih on 29.10.2016.
//

#pragma once

#include <GLFW/glfw3.h>

//CPP
#include <functional>
#include <vector>
#include "Drawable.h"

struct GLFWwindow;

namespace e2e
{
    class GUI;
    class Quad;

    class Window
    {
        friend class e2e::GUI;
        int m_width, m_height;
        GLFWwindow* m_window;
        GLFWmonitor* m_monitor;
        std::string m_name;

    public:
        Window(int width, int height, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr, const std::string& name = "E2EHDR");
        void Loop(const std::vector<std::reference_wrapper<Quad>>& quads);
        void Loop(const std::vector<std::reference_wrapper<drawable_base>>& drawables);
        void reset_viewport(GLFWwindow* wind);

        /*template <class... Ts>
        void Loop(Ts&&... drawables)
        {
            //INPUT//
            //Poll events before handling.
            glfwPollEvents();

            if (get_key_down(GLFW_KEY_ESCAPE))
            {
                glfwSetWindowShouldClose(m_window, GL_TRUE);
            }

            //RENDER//
            glClear(GL_COLOR_BUFFER_BIT);
            std::initializer_list<int> x { 0, (drawables.draw(), 0)... };
            (void)x;

            glfwSwapBuffers(m_window);
        }*/

        bool get_key_down(int btn);
        bool get_key_up(int btn);

        void ShouldClose(bool set);
        bool ShouldClose() const;

        void StartDraw();
        void EndDraw();

        GLFWwindow* get_window() const { return m_window; }
    };
}