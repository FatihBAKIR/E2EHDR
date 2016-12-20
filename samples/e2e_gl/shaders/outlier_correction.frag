#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;

void main()
{
	vec4 pixel = texture(disparity_map, tex_coord);
	
	//If this pixel is not an outlier.
	if (pixel.r-pixel.g<0.5f)
	{
		color = pixel;
		return;
	}
	
	//If it is an outlier.
	int closest_right = 1000000; //Large number required.
	vec4 right_scanline_pixel;
	for (int i=10; i<1280; i+=10)
	{
		right_scanline_pixel = texture(disparity_map, vec2(tex_coord.x+i*dx, tex_coord.y));
		if (right_scanline_pixel.r-right_scanline_pixel.g<0.5f)
		{
			for (int j=i-9; j<=i; ++j)
			{
				right_scanline_pixel = texture(disparity_map, vec2(tex_coord.x+j*dx, tex_coord.y));
				if (right_scanline_pixel.r-right_scanline_pixel.g<0.5f)
				{
					closest_right = i;
					break;
				}
			}
			break;
		}
	}
	
	int closest_left = 1000000; //Large number required.
	vec4 left_scanline_pixel;
	for (int i=-10; i>-1280; i-=10)
	{
		left_scanline_pixel = texture(disparity_map, vec2(tex_coord.x+i*dx, tex_coord.y));
		if (left_scanline_pixel.r-left_scanline_pixel.g<0.5f)
		{
			for (int j=i+9; j>=i; --j)
			{
				left_scanline_pixel = texture(disparity_map, vec2(tex_coord.x+j*dx, tex_coord.y));
				if (left_scanline_pixel.r-left_scanline_pixel.g<0.5f)
				{
					closest_left = i;
					break;
				}
			}
			break;
		}
	}
	
	if (closest_right<abs(closest_left))
	{
		color = right_scanline_pixel;
	}
	
	else
	{
		color = left_scanline_pixel;
	}
	
	/*float min_cost_disparity = round(texture(disparity_map, tex_coord).g*255 + 0.1f);
	float second_min_cost_disparity = round(texture(disparity_map, tex_coord).b*255 + 0.1f);
	
	//Early leave.
	if (min_cost_disparity==second_min_cost_disparity)
	{
		color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		return;
	}
	
	//Outlier detection
	float min_cost = 0.0f;
	float second_min_cost = 0.0f;
	for (int i=-5; i<=5; ++i)
	{
		for (int j=-5; j<=5; ++j)
		{
			min_cost += texture(dsi, vec3(tex_coord.x+i*dx, tex_coord.y+j*dy, min_cost_disparity)).r;
			second_min_cost += texture(dsi, vec3(tex_coord.x+i*dx, tex_coord.y+j*dy, second_min_cost_disparity)).r;
		}
	}
	
	if (second_min_cost/min_cost < 1.10f)
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	
	else
	{
		color = vec4(texture(disparity_map, tex_coord).rrr*4, 1.0f);
	}*/
}