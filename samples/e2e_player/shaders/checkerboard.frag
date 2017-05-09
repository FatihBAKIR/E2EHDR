#version 330

in vec2 tex_coord;
out vec4 color;

const float num_x = 20.0f;
const float num_y = 20.0f;
const float dx = 1.0f / num_x;
const float dy = 1.0f / num_y;

void main()
{
    float value = mod(floor(tex_coord.x / dx) + floor(tex_coord.y / dy), 2.0f);
    value = dot(vec3(0.2989, 0.5890, 0.114), vec3(value));
    value = sqrt(value);

	color = vec4(vec3(value), 1.0f);
}