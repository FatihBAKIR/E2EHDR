#version 330 core

in vec2 tex_coord;
in vec3 avg;
out vec4 color;

uniform sampler2D frame;
uniform vec2 global_align;
uniform int is_left;
uniform sampler2D left_avg;
uniform sampler2D right_avg;

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

float luminance(vec3 color)
{
    //Assuming that input color is in linear sRGB color space.
    return dot(vec3(0.2126, 0.7152, 0.0722), color);
}

float weight(float val)
{
    float w;

    if (val <= 0.5)
    {
        w = val * 2.0;
    }
    else
    {
        w = (1.0 - val) * 2.0;
    }

    return w;
}

void main()
{
    vec2 loc = tex_coord;
    if (is_left == 0)
        loc += global_align;
    vec3 col = vec3(texture(frame, loc));

    vec3 ratio = vec3(1);

    vec3 av_left = vec3(texture(left_avg, vec2(0.5)));
    vec3 av_r = vec3(texture(right_avg, vec2(0.5)));

    if (is_left == 0)
    {
        ratio = (av_left )  / av_r;
    }

    color = vec4((apply_crf(col, param) / param.exposure), weight(luminance(col)));
}
