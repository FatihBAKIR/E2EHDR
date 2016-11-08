//OTHER
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

//FRAMEWORK
#include "glsl_program.h"
#include "gui.h"
#include "imgui_wrapper.h"
#include "quad.h"

int main()
{
	int screen_width = 1280;
	int screen_height = 720;

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
	glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLSLProgram hdr;
	hdr.attachShader(GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
	hdr.attachShader(GLSLProgram::FRAGMENT_SHADER, "shaders/hdr.frag");
	hdr.create();

	Quad quad;
	quad.create();
	//quad.addTexture(image, width, height);

	//GUI STEP 1//
	e2e::GUI& gui = e2e::GUI::getGUI();
	gui.initialize(window, true);

	while (!glfwWindowShouldClose(window))
	{
		//INPUT//
		//Poll events before handling.
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		//GUI STEP 2//
		gui.newFrame();

		//SOME GUI STUFF//
		static float exposure1 = 0.5f;
		static float exposure2 = 0.5f;
		gui::displayCameraControl(exposure1, exposure2);

		//RENDER//
		glClear(GL_COLOR_BUFFER_BIT);
		/*hdr.use();
		quad.draw(hdr);*/

		//GUI STEP 3//
		ImGui::Render();

		glfwSwapBuffers(window);
	}

	return 0;
}