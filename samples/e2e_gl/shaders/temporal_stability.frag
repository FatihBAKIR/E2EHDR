#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;

uniform sampler2D previous_frame;
uniform sampler2D new_frame;

void main()
{
	const float avg_threshold = 0.04f; //Might be given via uniform variable.
	const float dev_threshold = 0.07f; //Might be given via uniform variable.
	
	float avg_difference = 0.0f;
	int diff_pixel_size = 0;
	for (int i=-3; i<=3; ++i)
	{
		for (int j=-3; j<=3; ++j)
		{
			vec4 current_pixel, new_pixel;
			current_pixel = texture(previous_frame, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
			new_pixel = texture(new_frame, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
			
			vec4 diff = abs(current_pixel-new_pixel);
			float curr_diff = dot(diff.rgb, vec3(0.33f));
			avg_difference += curr_diff;
		}
	}
	
	avg_difference /= 49.0f;
	
	//Calculate variance
	float variance = 0.0f;
	for (int i=-3; i<=3; ++i)
	{
		for (int j=-3; j<=3; ++j)
		{
			vec4 current_pixel, new_pixel;
			current_pixel = texture(previous_frame, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
			new_pixel = texture(new_frame, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
			
			vec4 diff = abs(current_pixel-new_pixel);
			float curr_diff = avg_difference - dot(diff.rgb, vec3(0.33f));
			
			variance += curr_diff * curr_diff;
		}
	}
	
	variance /= 49.0f;
	float deviation = sqrt(variance);
	
	if (avg_difference < avg_threshold && deviation < dev_threshold)
	{
		color = texture(previous_frame, tex_coord);
		return;
	}
	
	color = texture(new_frame, tex_coord);
}