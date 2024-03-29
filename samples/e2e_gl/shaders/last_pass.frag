#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D final_image;

void main()
{
    color = texture(final_image, tex_coord);
}
