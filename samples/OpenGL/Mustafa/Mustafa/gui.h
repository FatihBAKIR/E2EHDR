#pragma once

#include <string>

namespace gui
{
	void displayCameraControl(float& exposure1, float& exposure2);
	void displayPerformance(const std::string& window_name, double delta_time);
}