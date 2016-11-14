#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

void main()
{    
	color = texture(texture0, vec2(tex_coord.x, 1-tex_coord.y));
}