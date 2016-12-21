#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform int disparity_limit;
uniform float dx;
uniform float dy;

uniform sampler2D left;
uniform sampler2D right;
uniform sampler2DArray dsi;

void main()
{
	float window_min_cost=1.0f/0.0f; //Infinity
	int window_min_cost_index=0;
	
	float pixel_min_cost=1.0f/0.0f; //Infinity
	int pixel_min_cost_index=-1;
	int second_pixel_min_cost_index=0;
	for (int i=0; i<disparity_limit; ++i)
	{
		//Aggregate operations
		float current_cost=0.0f;
		for (int j=-1; j<=1; ++j)
		{
			for (int k=-1; k<=1; ++k)
			{
				current_cost += texture(dsi, vec3(tex_coord.x+j*dx, tex_coord.y+k*dy, i)).r;
			}
		}
		if (current_cost < window_min_cost)
		{
			window_min_cost = current_cost;
			window_min_cost_index=i;
		}
		
		//This operation should be performed regardless of what aggregation method is used
		//if outlier detection is wanted to be used.
		//Minimum cost for the particular pixel, not the whole window!
		float pixel_cost = texture(dsi, vec3(tex_coord.x, tex_coord.y, i)).r;
		if(pixel_cost<pixel_min_cost)
		{
			pixel_min_cost = pixel_cost;
			second_pixel_min_cost_index = pixel_min_cost_index;
			pixel_min_cost_index = i;
		}
	}
	
	if (second_pixel_min_cost_index==-1)
	{
		second_pixel_min_cost_index=pixel_min_cost_index;
	}
	
	float r=window_min_cost_index/255.0f;
	float g=pixel_min_cost_index/255.0f;
	float b=second_pixel_min_cost_index/255.0f;

	color = vec4(vec3(r, g, b), 1.0f);
}