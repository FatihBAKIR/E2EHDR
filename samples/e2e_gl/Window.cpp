//
// Created by fatih on 29.10.2016.
//
//GL headers.
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "quad.h"

namespace
{
    struct GLFW
    {
        GLFW()
        {
            auto ret = glfwInit();
            if (ret != GLFW_TRUE)
            {
                throw std::runtime_error("GLFW Init failed, amk cocugu");
            }
        }
        ~GLFW()
        {
            glfwTerminate();
        }
    };

    struct GLEW
    {
        GLEW()
        {
            //GLEW//
            //Initialize glew.
            //Before initializing it, make sure glewExperimental is set GL_TRUE.
            //To understand the reason, visit glew documentation.
            glewExperimental = GL_TRUE;
            auto res = glewInit();
            if (res != GLEW_OK)
            {
                throw std::runtime_error("GLEW Initialization Failed!");
            }
        }
    };
}

namespace e2e
{
    Window::Window(int width, int height)
            : w(width), h(height)
    {
        static GLFW glfw_global;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed

        //Create a window.
        window = glfwCreateWindow(w, h, "E2EHDR", nullptr, nullptr);
        assert(window);
        glfwMakeContextCurrent(window);

        GLEW g;

        //OpenGL//
        //Lower left point is (0,0). Top right point is (g_SCREEN_WIDTH, g_SCREEN_HEIGHT).
        glViewport(0, 0, w * 2, h * 2);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::Loop(const std::vector<std::reference_wrapper<Quad>>& quads)
    {
        /*INPUT*/
        //Poll events before handling.
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        /*RENDER*/
        glClear(GL_COLOR_BUFFER_BIT);
        for (auto&& q : quads)
        {
            q.get().draw();
        }
        glfwSwapBuffers(window);
    }

    bool Window::ShouldClose() const
    {
        return static_cast<bool>(glfwWindowShouldClose(window));
    }
}
