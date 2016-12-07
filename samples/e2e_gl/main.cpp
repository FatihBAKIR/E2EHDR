//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>
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

#define DISPARITY_LIMIT 64
#define IMAGE_WIDTH 450
#define IMAGE_HEIGHT 375

int main()
{
	e2e::Window w(IMAGE_WIDTH, IMAGE_HEIGHT);

	e2e::Quad quad;
	quad.create();

	int width, height;
	unsigned char* image1 = SOIL_load_image("teddy_left.png", &width, &height, 0, SOIL_LOAD_RGB);
	Texture tex1;
	tex1.load(image1, width, height);

	unsigned char* image2 = SOIL_load_image("teddy_right.png", &width, &height, 0, SOIL_LOAD_RGB);
	Texture tex2;
	tex2.load(image2, width, height);

	quad.set_textures(tex1, tex2);
	quad.set_position(0.0f, 0.0f);
	quad.set_scale_factor(1.0f, 1.0f);
	e2e::Merger merger(IMAGE_WIDTH, IMAGE_HEIGHT, DISPARITY_LIMIT);
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
		std::cout << elapsed*1000 <<" ms"<< std::endl;
	}

	return 0;
}