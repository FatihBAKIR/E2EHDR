#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform int disparity_level;
uniform float dx; //instead of uniform, can be made immediate value
uniform float dy; //instead of uniform, can be made immediate value

uniform sampler2D left;
uniform sampler2D right;

void main()
{
	vec4 left_pixel = texture(left, vec2(tex_coord.x, 1.0f-tex_coord.y));
	vec4 right_pixel = texture(right, vec2(tex_coord.x-dx*disparity_level, 1.0f-tex_coord.y));
	
	//CENSUS
	float hamming_distance = 0.0f;
	float left_pixel_intensity = dot(left_pixel.rgb, vec3(0.33f));
	float right_pixel_intensity = dot(right_pixel.rgb, vec3(0.33f));
	for (int i=-2; i<=2; ++i)
	{
		for (int j=-2; j<=2; ++j)
		{
			vec4 n_pixel;
			float n_intensity, comp_left, comp_right;
			
			n_pixel = texture(left, vec2(tex_coord.x+i*dx, 1.0f-(tex_coord.y+j*dy)));
			n_intensity = dot(n_pixel.rgb, vec3(0.33f));
			comp_left = step(0.0f, n_intensity-left_pixel_intensity);
			
			n_pixel = texture(right, vec2(tex_coord.x+(i-disparity_level)*dx, 1.0f-(tex_coord.y+j*dy)));
			n_intensity= dot(n_pixel.rgb, vec3(0.33f));
			comp_right = step(0.0f, n_intensity-right_pixel_intensity);
			
			hamming_distance += abs(sign(comp_left-comp_right));
		}
	}
	
	color = vec4(vec3(hamming_distance/25.0f), 1.0f);
}