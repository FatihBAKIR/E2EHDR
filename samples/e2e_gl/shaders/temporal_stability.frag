#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;

uniform sampler2D current_frame;
uniform sampler2D new_frame;

void main()
{
	const float threshold = 0.1f; //Might be given via uniform variable.
	
	float difference = 0.0f;
	for (int i=-2; i<=2; ++i)
	{
		for (int j=-2; j<=2; ++j)
		{
			vec4 current_pixel, new_pixel;
			current_pixel = texture(current_frame, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
			new_pixel = texture(new_frame, vec2(tex_coord.x+i*dx, tex_coord.y+j*dy));
			
			vec4 diff = abs(current_pixel-new_pixel);
			difference += dot(diff.rgb, 0.33f);
		}
	}
	
	difference/=25.0f;
	
	if (difference < threshold)
	{
		color = texture(current_frame, tex_coord);
	}
	
	color = texture(new_frame, tex_coord);
}