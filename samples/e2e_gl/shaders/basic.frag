#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;
uniform sampler2D left_exp;
uniform sampler2D right_exp;

struct undistort
{
    vec2 focal_length;
    vec2 optical_center;
    vec2 image_size;

    float dist_coeffs[5];
};

struct crf
{
    float red[256];
    float green[256];
    float blue[256];
};

struct camera_params
{
    crf         response;
    undistort   undis;
    float       exposure;
};

uniform camera_params left;
uniform camera_params right;

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

vec2 applyDisparity(vec2 point)     //TODO: implement
{
    return point;
}

float luminance(vec3 color)
{
    // Assuming that input color is in linear sRGB color space.

    return (color.r * 0.2126) +
           (color.g * 0.7152) +
           (color.b * 0.0722);
}

float weight(float val)
{
    float w;

    if (val <= 0.5){
        w = val * 2.0;
    }
    else {
        w = (1.0 - val) * 2.0;
    }

    return w;
}

vec4 createHDR(vec2 resultUV)
{
    float weightSum = 0.0;
    vec4 hdr;

    // frame with lower exposure:
    vec3  ldr_left      = texture(left_exp, vec2(resultUV.x, resultUV.y)).rgb;
    float lum_left      = luminance(ldr_left);
    float w_left        = weight(lum_left);
    float exp_left      = left.exposure;

    hdr.rgb       += (ldr_left / exp_left) * w_left;
    weightSum     += w_left;

    // frame with higher exposure:
    vec3  ldr_right      = texture(right_exp, applyDisparity(vec2(resultUV.x, resultUV.y))).rgb;
    float lum_right      = luminance(ldr_right);
    float w_right        = weight(lum_right);
    float exp_right      = right.exposure;

    hdr.rgb       += (ldr_right / exp_right) * w_right;
    weightSum     += w_right;

    // overall value:
    hdr.rgb /= weightSum + 1e-6;
    hdr.a    = log(luminance(hdr.rgb) + 1e-6);

    return hdr; // returns the rgba value.
}

void main()
{
    vec2 resultUV = tex_coord;
    resultUV.y = 1 - resultUV.y;

    vec4 cols = createHDR(resultUV);

    cols = pow(cols, vec4(1.0 / 2.2)); // gamma correction

    color = cols;
}