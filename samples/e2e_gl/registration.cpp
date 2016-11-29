#include "registration.h"
#include <algorithm>

namespace e2e
{
	DisparityMap::DisparityMap(const Image& left, const Image& right)
		: m_left(left)
		, m_right(right)
	{
		auto width = left.m_width;
		auto height = left.m_height;

		for (int k = 0; k < DISPARITY_LEVEL_LIMIT; ++k)
		{
			m_dsi[k].resize(width);

			for (auto& col : m_dsi[k])
			{
				col.resize(height);
			}
		}
	}

	void DisparityMap::computeCost()
	{
		int width = m_left.m_width;
		int height = m_left.m_height;

		//A simple absolute difference cost.
		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				for (int d = 0; d < DISPARITY_LEVEL_LIMIT; ++d)
				{
					auto left_pixel = m_left.pixel(i, j);
					auto index_i = std::max(i - d, 0);
					auto right_pixel = m_right.pixel(index_i, j);
					int diff = std::abs(static_cast<int>(left_pixel[0]) - static_cast<int>(right_pixel[0]))
						+ std::abs(static_cast<int>(left_pixel[1]) - static_cast<int>(right_pixel[1]))
						+ std::abs(static_cast<int>(left_pixel[2]) - static_cast<int>(right_pixel[2]));

					//Diff can be minimum 0 and maximum 765. Normalize it to [0,1]
					m_dsi[d][i][j] = diff / 765.0f;
				}
			}
		}
	}

	void DisparityMap::aggregate()
	{
		int width = m_left.m_width;
		int height = m_left.m_height;
		float wta[DISPARITY_LEVEL_LIMIT];

		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				for (int d = 0; d < DISPARITY_LEVEL_LIMIT; ++d)
				{

				}
			}
		}
	}

	void DisparityMap::warp()
	{
		//TODO
	}

	unsigned char* DisparityMap::getDisparityMap()
	{
		int width = m_left.m_width;
		int height = m_left.m_height;

		auto ptr = new unsigned char[width*height * 3];
		Image image(ptr, width, height);

		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				auto pixel1 = image.pixel(i, j);
				auto pixel2 = m_left.pixel(i, j);
				pixel1[0] = pixel2[0];
				pixel1[1] = pixel2[1];
				pixel1[2] = pixel2[2];
			}
		}

		return ptr;
	}
}