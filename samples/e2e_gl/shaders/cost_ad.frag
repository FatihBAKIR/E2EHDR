#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform int disparity_level;
uniform float dx;

uniform sampler2D left;
uniform sampler2D right;

void main()
{
	//absolute difference
	vec4 left_pixel = texture(left, vec2(tex_coord.x, 1.0f-tex_coord.y));
	vec4 right_pixel = texture(right, vec2(tex_coord.x-dx*disparity_level, 1.0f-tex_coord.y));
	
	vec4 absolute_difference = abs(left_pixel-right_pixel);
	
	float average = (absolute_difference.r+absolute_difference.g+absolute_difference.b)/3;
	
	color = vec4(vec3(average), 1.0f);
}