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
	float min_cost=1.0f/0.0f; //Infinity
	int min_cost_index=0;
	const float threshhold= 0.08f;
	
	vec4 main_pixel = texture(left, tex_coord);
	for (int i=0; i<disparity_limit; ++i)
	{
		float current_cost=0.0f;
		for (int j=-2; j<=2; ++j)
		{
			for (int k=-2; k<=2; ++k)
			{
				vec4 curren_pixel = texture(left, vec2(tex_coord.x*j*dx, tex_coord.y*k*dy));
				vec4 diff = abs(main_pixel-curren_pixel);
				
				vec4 pixel_cost = texture(dsi, vec3(tex_coord.x+j*dx, tex_coord.y+k*dy, i));
				if (max(max(diff.r, diff.g), diff.b) >= threshhold && pixel.r-pixel.g < 0.5f)
				{
					current_cost += pixel_cost.r;
				}
			}
		}
		if (current_cost < min_cost)
		{
			min_cost = current_cost;
			min_cost_index=i;
		}
	}
	
	float val=min_cost_index/((disparity_limit-1)*1.0f);
	color = vec4(vec3(val), 1.0f);
}