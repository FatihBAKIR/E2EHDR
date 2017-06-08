#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture_coordinate;

out vec2 tex_coord;

uniform vec2 scale;
uniform vec2 translate;

uniform sampler2D frame;
uniform sampler2D other;

uniform int is_left;

out vec3 avg;

struct crf
{
    float red[256];
    float green[256];
    float blue[256];
};

struct camera_params
{
    crf         response;
    float       exposure;
};

uniform camera_params param;

int get_byte(float channel)
{
    return min(int(channel * 255), 255);
}

vec3 apply_crf(vec3 col, camera_params camera)
{
    float r = camera.response.red[get_byte(col.r)];
    float g = camera.response.green[get_byte(col.g)];
    float b = camera.response.blue[get_byte(col.b)];

    return vec3(r, g, b);
}

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

    avg = apply_crf(vec3(texture(frame, vec2(0.5f, 0.5f), 100)), param);
}
