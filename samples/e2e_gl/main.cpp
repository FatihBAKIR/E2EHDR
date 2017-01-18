//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include "include\glad\glad.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

//FRAMEWORK
#include "glsl_program.h"
#include "quad.h"
#include "gui.h"
#include "imgui_wrapper.h"
#include "merger.h"
#include "texture.h"
#include "Window.h"

//OTHER
#include <SOIL.h>

//CPP
#include <chrono>
#include <iostream>

#include "Drawable.h"

using namespace e2e;

constexpr int gDisparityLimit = 128;
constexpr int gScreenWidth = 1280;
constexpr int gScreenHeight = 720;

int main()
{
	e2e::Window w(gScreenWidth, gScreenHeight);

	int width, height;
	unsigned char* image1 = SOIL_load_image("left_im.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	Texture tex1;
	tex1.create(width, height, image1);

	unsigned char* image2 = SOIL_load_image("right_im.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	Texture tex2;
	tex2.create(width, height, image2);

	e2e::Merger merger(width, height, gDisparityLimit);
	merger.set_textures(tex1, tex2);
	merger.set_position(-0.5f, 0.0f);
	merger.set_scale_factor(0.5f, 0.5f);

	e2e::GUI& gui = e2e::GUI::getGUI();
	gui.initialize(w, true);
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

		glClear(GL_COLOR_BUFFER_BIT);
		gui.newFrame();

		static bool recompile_shaders = false;
		static int cost_choice = 1;
		static int agg_choice = 0;
		static bool detection = false;
		static bool correction = false;
		static bool median = false;
		static float threshold = 1.10;
		static int window_size = 7;

		e2e::gui::displayStereoControl(recompile_shaders, cost_choice, agg_choice, detection, correction, median, threshold, window_size);
		merger.chooseCost(cost_choice);
		merger.chooseAggregation(agg_choice);
		merger.set_outlier_detection(detection, threshold, window_size);
		merger.set_outlier_correction(correction);
		merger.set_median_filter(median);

		if (recompile_shaders)
		{
			merger.compileShaders();
		}

		merger.draw();

		ImGui::Render();
		glfwSwapBuffers(w.get_window());

		using second = std::chrono::duration<double, std::ratio <1>>;
		auto elapsed = std::chrono::duration_cast<second>(std::chrono::system_clock::now() - m_start_time).count();
		std::cout << elapsed * 1000 << " ms" << std::endl;
	}

	return 0;
}