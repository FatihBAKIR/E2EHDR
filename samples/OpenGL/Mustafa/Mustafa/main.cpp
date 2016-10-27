//GL headers.
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//FRAMEWORK
#include "pipeline.h"
#include "quad.h"
#include <SOIL.h>

int main()
{
	int screen_width = 800;
	int screen_height = 600;

	//GLFW//
	//Initialize glfw.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//Create a window.
	auto window = glfwCreateWindow(screen_width, screen_height, "E2EHDR", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//GLEW//
	//Initialize glew.
	//Before initializing it, make sure glewExperimental is set GL_TRUE.
	//To understand the reason, visit glew documentation.
	glewExperimental = GL_TRUE;
	glewInit();

	//OpenGL//
	//Lower left point is (0,0). Top right point is (g_SCREEN_WIDTH, g_SCREEN_HEIGHT).
	glViewport(0, 0, screen_width, screen_height);
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Pipeline hdr;
	hdr.attachShader(Pipeline::VERTEX_SHADER, "shaders/hdr.vert");
	hdr.attachShader(Pipeline::FRAGMENT_SHADER, "shaders/hdr.frag");
	hdr.create();

	Quad quad;
	quad.create();
	//quad.addTexture(image, width, height);

	while (!glfwWindowShouldClose(window))
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
		hdr.use();
		quad.draw(hdr);
		glfwSwapBuffers(window);
	}

	return 0;
}