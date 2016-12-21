#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform int N;
uniform float threshold;
uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;
uniform sampler2DArray dsi;

void main()
{
	float min_cost_disparity = round(texture(disparity_map, tex_coord).g*255 + 0.1f);
	float second_min_cost_disparity = round(texture(disparity_map, tex_coord).b*255 + 0.1f);
	
	//Early leave.
	if (min_cost_disparity==second_min_cost_disparity)
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		return;
	}
	
	//Outlier detection
	float min_cost = 0.0f;
	float second_min_cost = 0.0f;
	for (int i=-N; i<=N; ++i)
	{
		for (int j=-N; j<=N; ++j)
		{
			min_cost += texture(dsi, vec3(tex_coord.x+i*dx, tex_coord.y+j*dy, min_cost_disparity)).r;
			second_min_cost += texture(dsi, vec3(tex_coord.x+i*dx, tex_coord.y+j*dy, second_min_cost_disparity)).r;
		}
	}
	
	if (second_min_cost/min_cost < threshold)
	{
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	
	else
	{
		color = vec4(texture(disparity_map, tex_coord).rrr*4, 1.0f);
	}
}