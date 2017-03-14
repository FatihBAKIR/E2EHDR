#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture_coordinate;

out vec2 tex_coord;
out vec3 ratio;

uniform vec2 scale;
uniform vec2 translate;

uniform sampler2D frame;
uniform sampler2D other;

uniform int is_left;

void main()
{
    float x = position.x * scale.x + translate.x;
    float y = position.y * scale.y + translate.y;
    gl_Position = vec4(x, y, 0.0f, 1.0f);
	
	if (is_left == 1)
		ratio = vec3(1.0, 1.0, 1.0);
	else
		ratio = vec3(texture(frame, vec2(0.5, 0.5), 10.0) / texture(other, vec2(0.5, 0.5), 10.0));
		
    tex_coord = texture_coordinate;
}
