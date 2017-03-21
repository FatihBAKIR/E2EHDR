//
// Created by fatih on 29.10.2016.
//

//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
//#include "include\glad\glad.h"
#include "include/glad/glad.h"
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

	void glad_post_handler(const char *name, void *funcptr, int len_args, ...)
	{
		GLenum error_code;
		error_code = glad_glGetError();

		if (error_code != GL_NO_ERROR)
		{
			throw std::runtime_error("something broke, fuck it (" + std::string(name) + ")");
		}
	}
}

namespace e2e
{
	Window::Window(int width, int height, bool ts)
		: twoscreens(ts),
		  m_width(width),
		  m_height(height)
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


		if (!twoscreens)
		{
			m_window_primary = glfwCreateWindow(m_width, m_height, "E2EHDR", nullptr, nullptr);
			m_window_secondary = nullptr;
		}
		else
		{
			auto primary = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primary);

			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	    	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

//			m_width = mode->width;
//			m_height = mode->height;

			m_window_primary   = glfwCreateWindow(m_width, m_height, "E2EHDRPrimary", nullptr, nullptr);
			m_window_secondary = glfwCreateWindow(m_width, m_height, "E2EHDRSecondary", nullptr, m_window_primary);
		}

		assert(m_window_primary);
		glfwMakeContextCurrent(m_window_primary);

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
		reset_viewport();

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
			glfwSetWindowShouldClose(m_window_primary, GL_TRUE);
			if (twoscreens)
				glfwSetWindowShouldClose(m_window_secondary, GL_TRUE);
		}

		//RENDER//
		glClear(GL_COLOR_BUFFER_BIT);
		for (auto&& q : quads)
		{
			q.get().draw();
		}
		glfwSwapBuffers(m_window_primary);

		if (twoscreens)
			glfwSwapBuffers(m_window_secondary);
	}

	void Window::reset_viewport()
	{
	    int vp_w, vp_h;
	    glfwGetFramebufferSize(m_window_primary, &vp_w, &vp_h);
		glViewport(0, 0, vp_w, vp_h);
	}

	bool Window::get_key_up(int key)
	{
		return glfwGetKey(m_window_primary, key) == GLFW_RELEASE;
	}

	bool Window::get_key_down(int key)
	{
		return glfwGetKey(m_window_primary, key) == GLFW_PRESS;
	}


	bool Window::ShouldClose() const
	{
		return static_cast<bool>(glfwWindowShouldClose(m_window_primary));
	}

    void Window::StartDraw()
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::EndDraw()
    {
		glfwSwapBuffers(m_window_primary);
		if (twoscreens)
			glfwSwapBuffers(m_window_secondary);
    }

    void Window::Loop(const std::vector<std::reference_wrapper<drawable_base>> &drawables)
    {
        //INPUT//
        //Poll events before handling.
        glfwPollEvents();

        if (get_key_down(GLFW_KEY_ESCAPE))
        {
			glfwSetWindowShouldClose(m_window_primary, GL_TRUE);
			if (twoscreens)
				glfwSetWindowShouldClose(m_window_secondary, GL_TRUE);
        }

        //RENDER//
        glClear(GL_COLOR_BUFFER_BIT);
        for (auto& drawable : drawables)
        {
            drawable.get().draw();
        }
		glfwSwapBuffers(m_window_primary);
		if (twoscreens)
			glfwSwapBuffers(m_window_secondary);
    }

    void Window::ShouldClose(bool set) {
		glfwSetWindowShouldClose(m_window_primary, set);
		if (twoscreens)
			glfwSetWindowShouldClose(m_window_secondary, set);
    }
}