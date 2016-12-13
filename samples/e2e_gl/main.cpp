//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>g
#include <GLFW/glfw3.h>

//FRAMEWORK
#include "glsl_program.h"
#include "quad.h"
#include "merger.h"
#include "Window.h"

//OTHER
#include <SOIL.h>

//CPP
#include <chrono>
#include <iostream>

#include "Drawable.h"

using namespace e2e;

constexpr int gDisparityLimit = 64;
constexpr int gImageWidth = 450;
constexpr int gImageHeight = 375;

int main()
{
	e2e::Window w(gImageWidth, gImageHeight);

	int width, height;
	unsigned char* image1 = SOIL_load_image("cones_left.png", &width, &height, 0, SOIL_LOAD_RGB);
	Texture tex1;
	tex1.load(image1, width, height);

	unsigned char* image2 = SOIL_load_image("cones_right.png", &width, &height, 0, SOIL_LOAD_RGB);
	Texture tex2;
	tex2.load(image2, width, height);

	e2e::Merger merger(gImageWidth, gImageHeight, gDisparityLimit);
	merger.set_textures(tex1, tex2);

	while (!w.ShouldClose())
	{
		auto m_start_time = std::chrono::system_clock::now();
		//INPUT//
		//Poll events before handling.
		glfwPollEvents();

		if (glfwGetKey(w.get_window(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(w.get_window(), GL_TRUE);
		}

		merger.draw();

		glfwSwapBuffers(w.get_window());

		using second = std::chrono::duration<double, std::ratio <1>>;
		auto elapsed = std::chrono::duration_cast<second>(std::chrono::system_clock::now() - m_start_time).count();
		std::cout << elapsed * 1000 << " ms" << std::endl;
	}

	return 0;
}