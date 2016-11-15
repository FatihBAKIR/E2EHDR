//OTHER
#include <imgui.h>

//FRAMEWORK
#include "gui.h"

//CPP
#include <cmath>
#include <stdio.h>
#include <vector>

#define ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

namespace gui
{
	//Argument might be given as a structure such as CameraControl in the future.
	void displayCameraControl(float& exposure1, float& exposure2)
	{
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		//ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		static bool open1 = true;
		if (!ImGui::Begin("Camera 1", &open1, window_flags))
		{
			ImGui::End();
			return;
		}

		static bool auto_exposure1 = false;
		ImGui::Checkbox("Auto Exposure", &auto_exposure1);
		if (!auto_exposure1)
		{
			ImGui::VSliderFloat("Exposure Time", ImVec2(100.0f, 250.0f), &exposure1, 0.0f, 1.0f);
		}
		ImGui::End();



		//ImGui::SetNextWindowPos(ImVec2(0.0f, 400.0f));
		static bool open2 = true;
		if (!ImGui::Begin("Camera 2", &open2, window_flags))
		{
			ImGui::End();
			return;
		}

		static bool auto_exposure2 = false;
		ImGui::Checkbox("Auto Exposure", &auto_exposure2);
		if (!auto_exposure2)
		{
			ImGui::VSliderFloat("Exposure Time", ImVec2(100.0f, 250.0f), &exposure2, 0.0f, 1.0f);
		}
		ImGui::End();
	}

	void displayPerformance(const std::string& window_name, double delta_time)
	{
		static int index = 0;
		static std::vector<float> delta_times(100, 0.0);
		delta_times[index++] = delta_time;
		index %= 100;

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		static bool open = true;
		if (!ImGui::Begin(window_name.c_str(), &open, window_flags))
		{
			ImGui::End();
			return;
		}

		static bool fps_or_mspf = true;
		ImGui::Checkbox("FPS/MSPF", &fps_or_mspf);

		float arr[10] = { 0 };
		int index2 = 0;
		for (int i = index/10; i < index/10+10; ++i)
		{
			for (int j = (i%10) * 10; j < (i % 10) * 10 + 10; ++j)
			{
				arr[index2] += delta_times[j];
			}

			if (fps_or_mspf)
			{
				arr[index2] *= 1000;
			}

			else
			{
				arr[index2] = 1.0 / arr[index2];
			}

			++index2;
		}

		ImGui::PlotLines("", arr, ARRAYSIZE(arr));

		ImGui::End();
	}
}