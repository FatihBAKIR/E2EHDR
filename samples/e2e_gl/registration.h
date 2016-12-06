#pragma once

#define DISPARITY_LEVEL_LIMIT 200

#include <vector>

namespace e2e
{
	struct Image
	{
		unsigned char* m_image;
		int m_width;
		int m_height;

		Image(unsigned char* image, int width, int height)
			: m_image(image)
			, m_width(width)
			, m_height(height)
		{}

		inline unsigned char* pixel(int x, int y)
		{
			return (m_image+y*m_width*3 + x*3);
		}
	};

	class DisparityMap
	{
	public:
		DisparityMap(const Image& left, const Image& right);
		//~DisparityMap();

		void computeCost();
		void aggregate();
		void warp();

		unsigned char* getDisparityMap();

	private:
		//Disparity Space Image
		using CostMap = std::vector<std::vector<float>>;
		CostMap m_dsi[DISPARITY_LEVEL_LIMIT];

		Image m_left;
		Image m_right;
	};
}