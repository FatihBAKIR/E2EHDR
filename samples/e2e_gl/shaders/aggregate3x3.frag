#version 430 core

#define DISPARITY_LIMIT 64

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;

uniform sampler2D left;
uniform sampler2D right;
uniform sampler2DArray dsi;

void main()
{
	float min_cost=1.0f/0.0f; //Infinity
	int min_cost_index=0;
	for (int i=0; i<DISPARITY_LIMIT; ++i)
	{
		float current_cost=0.0f;
		for (int j=-1; j<=1; ++j)
		{
			for (int k=-1; k<=1; ++k)
			{
				current_cost += texture(dsi, vec3(tex_coord.x+j*dx, tex_coord.y+k*dy, i)).r;
			}
		}
		if (current_cost < min_cost)
		{
			min_cost = current_cost;
			min_cost_index=i;
		}
	}
	
	float val=min_cost_index/((DISPARITY_LIMIT-1)*1.0f);
	color = vec4(vec3(val), 1.0f);
	
	/*float color_r = texture(left, vec2(tex_coord.x, 1-tex_coord.y)).r;
	float color_g = texture(right, vec2(tex_coord.x, 1-tex_coord.y)).g;
	color = vec4(color_r, color_g, 0.0f, 1.0f);*/
}