#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D source_texture;

void main()
{
	color = texture(source_texture, tex_coord);
}