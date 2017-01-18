#pragma once

namespace e2e
{
	namespace gui
	{
		void displayCameraControl(float& exposure1, float& exposure2);
		void displayStereoControl(bool& recompile_shaders, int& cost_choice, int& agg_choice, bool& detection, bool& correction, bool& median, float& threshold, int& window_size);
		void displayTonemapControl(float& base_lum, float& max_lum);
	}
}