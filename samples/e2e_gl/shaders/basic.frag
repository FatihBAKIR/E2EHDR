#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;

void main()
{
	color = texture(disparity_map, tex_coord);
}