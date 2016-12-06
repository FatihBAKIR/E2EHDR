//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//FRAMEWORK
#include "glsl_program.h"
#include "quad.h"
#include "registration.h"
#include "Window.h"

//OTHER
#include <SOIL.h>

//CPP
#include <chrono>
#include <iostream>

using namespace e2e;

#define DISPARITY_LIMIT 64
#define IMAGE_WIDTH 450
#define IMAGE_HEIGHT 375

int main()
{
	e2e::Window w(IMAGE_WIDTH, IMAGE_HEIGHT);

	e2e::GLSLProgram cost;
	cost.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
	cost.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/cost_adcensus.frag");
	cost.link();

	e2e::GLSLProgram aggregate;
	aggregate.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
	aggregate.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/aggregate3x3.frag");
	aggregate.link();

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

	//FRAMEBUFFER
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	//ARRAY TEXTURE
	GLuint array_texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &array_texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, IMAGE_WIDTH, IMAGE_HEIGHT, DISPARITY_LIMIT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

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

		//RENDER//
		//MULTIPASS TO ARRAY TEXTURE
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT);

		quad.set_program(cost);
		cost.setUniformFVar("dx", { 1.0f / IMAGE_WIDTH });
		cost.setUniformFVar("dy", { 1.0f / IMAGE_HEIGHT });
		for (GLint i = 0; i < DISPARITY_LIMIT; ++i)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
			cost.setUniformIVar("disparity_level", { i });
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, array_texture, 0, i);

			//Draw quad
			quad.draw();
		}

		//SECOND PASS
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		quad.set_program(aggregate);
		aggregate.setUniformFVar("dx", { 1.0f / IMAGE_WIDTH });
		aggregate.setUniformFVar("dy", { 1.0f / IMAGE_HEIGHT });
		glActiveTexture(GL_TEXTURE2);
		aggregate.setUniformIVar("dsi", { 2 });
		glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);

		//Draw quad
		quad.draw();

		glfwSwapBuffers(w.get_window());

		using second = std::chrono::duration<double, std::ratio <1>>;
		auto elapsed = std::chrono::duration_cast<second>(std::chrono::system_clock::now() - m_start_time).count();
		std::cout << elapsed*1000 <<" ms"<< std::endl;
	}

	return 0;
}