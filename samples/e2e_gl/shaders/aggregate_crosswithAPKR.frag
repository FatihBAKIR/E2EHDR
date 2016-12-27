#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform int disparity_limit;
uniform float dx;
uniform float dy;

uniform sampler2D left;
uniform sampler2D right;
uniform sampler2DArray dsi;

#define L 2

void main()
{
	float min_cost=1.0f/0.0f; //Infinity
	int min_cost_index=0;
	
	const float threshhold = 0.08f;
	int top_bound=L;
	int bottom_bound=-L;
	
	//bound[k].x represents the left bound of a vertical arm pixel
	//bound[k].y represents the right bound of a vertical arm pixel
	ivec2 bounds[2*L+1];
	
	//Calculate bounds
	//Up
	vec4 main_pixel = texture(left, vec2(tex_coord.x, tex_coord.y));
	for (int j=0; j<=L; ++j)
	{
		vec4 arm_pixel = texture(left, vec2(tex_coord.x, tex_coord.y+j*dy));
		vec4 diff = abs(main_pixel-arm_pixel);
		if (max(max(diff.r, diff.g), diff.b) < threshhold)
		{
			bounds[j+L].x = -L;
			bounds[j+L].y = L;
			
			//Right
			for (int k=0; k<=L; ++k)
			{
				vec4 hor_arm_pixel = texture(left, vec2(tex_coord.x+k*dx, tex_coord.y+j*dy));
				vec4 hor_diff = abs(main_pixel-hor_arm_pixel);
				if (max(max(hor_diff.r, hor_diff.g), hor_diff.b) >= threshhold)
				{
					bounds[j+L].y = k-1;
					break;
				}
			}
			
			//Left
			for (int k=-1; k>=-L; --k)
			{
				vec4 hor_arm_pixel = texture(left, vec2(tex_coord.x+k*dx, tex_coord.y+j*dy));
				vec4 hor_diff = abs(main_pixel-hor_arm_pixel);
				if (max(max(hor_diff.r, hor_diff.g), hor_diff.b) >= threshhold)
				{
					bounds[j+L].x = k+1;
					break;
				}
			}
		}
			
		else
		{
			top_bound = j-1;
			break;
		}
	}
	
	//Down
	for (int j=-1; j>=-L; --j)
	{
		vec4 arm_pixel = texture(left, vec2(tex_coord.x, tex_coord.y+j*dy));
		vec4 diff = abs(main_pixel-arm_pixel);
		if (max(max(diff.r, diff.g), diff.b) < threshhold)
		{
			bounds[j+L].x = -L;
			bounds[j+L].y = L;
			
			//Right
			for (int k=0; k<=L; ++k)
			{
				vec4 hor_arm_pixel = texture(left, vec2(tex_coord.x+k*dx, tex_coord.y+j*dy));
				vec4 hor_diff = abs(main_pixel-hor_arm_pixel);
				if (max(max(hor_diff.r, hor_diff.g), hor_diff.b) >= threshhold)
				{
					bounds[j+L].y = k-1;
					break;
				}
			}
			
			//Left
			for (int k=-1; k>=-L; --k)
			{
				vec4 hor_arm_pixel = texture(left, vec2(tex_coord.x+k*dx, tex_coord.y+j*dy));
				vec4 hor_diff = abs(main_pixel-hor_arm_pixel);
				if (max(max(hor_diff.r, hor_diff.g), hor_diff.b) >= threshhold)
				{
					bounds[j+L].x = k+1;
					break;
				}
			}
		}
			
		else
		{
			bottom_bound = j+1;
			break;
		}
	}
	
	float pixel_min_cost=1.0f/0.0f; //Infinity
	int pixel_min_cost_index=-1;
	int second_pixel_min_cost_index=0;
	for (int i=0; i<disparity_limit; ++i)
	{
		float current_cost=0.0f;
		
		for (int j=bottom_bound; j<=top_bound; ++j)
		{
			for (int k=bounds[j+L].x; k<=bounds[j+L].y; ++k)
			{
				current_cost += texture(dsi, vec3(tex_coord.x+k*dx, tex_coord.y+j*dy, i)).r;
			}
		}
		
		if (current_cost < min_cost)
		{
			min_cost = current_cost;
			min_cost_index=i;
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
	
	float r=min_cost_index/255.0f;
	float g=pixel_min_cost_index/255.0f;
	float b=second_pixel_min_cost_index/255.0f;

	color = vec4(vec3(r, g, b), 1.0f);
}