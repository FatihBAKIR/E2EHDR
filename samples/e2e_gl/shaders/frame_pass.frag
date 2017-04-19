#version 330 core

in vec2 tex_coord;
in vec3 ratio;
out vec4 color;

uniform sampler2D frame;

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
    vec3 col = vec3(texture(frame, tex_coord));
	
    color = vec4(apply_crf(col, param) / param.exposure * ratio, weight(luminance(col)));
}
