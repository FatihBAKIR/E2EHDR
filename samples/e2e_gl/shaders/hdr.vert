#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture_coordinate;

out vec2 tex_coord;

uniform vec2 scale;
uniform vec2 translate;
uniform int is_left;

void main()
{
    float x = position.x * scale.x + translate.x;
    float y = position.y * scale.y + translate.y;
    gl_Position = vec4(x, y, 0.0f, 1.0f);

    if (is_left == 1)
    {
        tex_coord = vec2(1.0f) - texture_coordinate;
    }
    else
    {
        tex_coord = texture_coordinate;
    }
}
