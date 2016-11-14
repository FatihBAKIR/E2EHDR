#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture_coordinate;

out vec2 tex_coord;

uniform vec2 scale;
uniform vec2 translate;

void main()
{
    float x = position.x * scale.x + translate.x;
    float y = position.y * scale.y + translate.y;
    gl_Position = vec4(x, y, 0.0f, 1.0f);
    tex_coord = texture_coordinate;
}