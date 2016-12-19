//
// Created by fatih on 29.10.2016.
//

//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
			if (res != GLFW_TRUE)
			{
				throw std::runtime_error("GLFW Initialization Failed!");
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
		: m_width(width)
		, m_height(height)
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

		m_window = glfwCreateWindow(m_width, m_height, "E2EHDR", nullptr, nullptr);
		assert(m_window);
		glfwMakeContextCurrent(m_window);

		//GLEW//
		GLEW g;

		//OpenGL//
		//Lower left point is (0,0). Top right point is (width, height).
#ifdef __APPLE__
		glViewport(0, 0, m_width * 2, m_height * 2);
#else
		glViewport(0, 0, m_width, m_height);
#endif

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
		glfwSwapBuffers(m_window);
	}

	void Window::reset_viewport()
	{
#ifdef __APPLE__
		glViewport(0, 0, m_width * 2, m_height * 2);
#else
		glViewport(0, 0, m_width, m_height);
#endif
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

    void Window::ShouldClose(bool set) {
        glfwSetWindowShouldClose(m_window, set);
    }
}