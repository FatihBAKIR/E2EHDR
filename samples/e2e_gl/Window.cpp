//
// Created by fatih on 29.10.2016.
//

//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
//#include "include\glad\glad.h"
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

//FRAMEWORK
#include "Window.h"
#include "quad.h"

namespace
{
	struct GLFW
	{
		GLFW()
		{
			auto res = glfwInit();
			if (res != 1)
			{
				throw std::runtime_error("GLFW Initialization Failed!");
			}
		}
		~GLFW()
		{
			glfwTerminate();
		}
	};

	void glad_post_handler(const char *name, void *funcptr, int len_args, ...)
	{
		GLenum error_code;
		error_code = glad_glGetError();

		if (error_code != GL_NO_ERROR)
		{
			throw std::runtime_error("something broke (" + std::string(name) + ")");
		}
	}
}

namespace e2e
{
	Window::Window(int width, int height, GLFWmonitor* monitor, GLFWwindow* share, const std::string& name)
        : m_width(width),
		  m_height(height),
          m_monitor(monitor),
          m_name(name)
	{
		//GLFW//
		static GLFW glfw_global;


		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        const GLFWvidmode* mode;
        if (m_monitor){
			mode = glfwGetVideoMode(monitor);
        }
        else {
            mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        }

        m_width = mode->width;
		m_height = mode->height;

        m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, share);

		assert(m_window);
		glfwMakeContextCurrent(m_window);

		static auto _ = ([&] {
			if (!gladLoadGL())
			{
				printf("Something went wrong!\n");
				exit(-1);
			}

			glad_set_post_callback(glad_post_handler);

			return 0;
		})();

		//OpenGL//
		//Lower left point is (0,0). Top right point is (width, height).
		reset_viewport(m_window);

		glfwMakeContextCurrent(m_window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Window::Loop(const std::vector<std::reference_wrapper<Quad>>& quads)
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
		for (auto&& q : quads)
		{
			q.get().draw();
		}
		EndDraw();
	}

	void Window::reset_viewport(GLFWwindow* wind)
	{
	    int vp_w, vp_h;
	    glfwGetFramebufferSize(wind, &vp_w, &vp_h);
		glfwMakeContextCurrent(wind);
		glViewport(0, 0, vp_w, vp_h);
	}

	bool Window::get_key_up(int key)
	{
		return glfwGetKey(m_window, key) == GLFW_RELEASE;
	}

	bool Window::get_key_down(int key)
	{
		return glfwGetKey(m_window, key) == GLFW_PRESS;
	}


	bool Window::ShouldClose() const
	{
		return static_cast<bool>(glfwWindowShouldClose(m_window));
	}

    void Window::StartDraw()
    {
        glfwPollEvents();
		glfwMakeContextCurrent(m_window);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::EndDraw()
    {
		glfwSwapBuffers(m_window);
    }

    void Window::Loop(const std::vector<std::reference_wrapper<drawable_base>> &drawables)
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
        for (auto& drawable : drawables)
        {
            drawable.get().draw();
        }
		glfwSwapBuffers(m_window);
    }

    void Window::ShouldClose(bool set)
	{
		glfwSetWindowShouldClose(m_window, set);
    }


    void Window::go_fullscreen(GLFWmonitor* mon)
    {

        const GLFWvidmode* mode;
        m_monitor = mon;
			mode = glfwGetVideoMode(mon);

        m_width = mode->width;
		m_height = mode->height;
		glfwSetWindowMonitor(m_window, mon, 0, 0, m_width, m_height, GLFW_DONT_CARE);
    }
}

