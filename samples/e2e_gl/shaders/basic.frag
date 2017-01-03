#version 430 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;
uniform sampler2D left_exp;
uniform sampler2D right_exp;

struct camera_params
{
    float       exposure;
};

uniform camera_params left;
uniform camera_params right;

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
    vec4 hdr = vec4(0.0, 0.0, 0.0, 1.0);

    // frame with lower exposure:
    vec3  ldr_left      = texture(left_exp, resultUV).rgb;
    float lum_left      = luminance(ldr_left);
    float w_left        = weight(lum_left); 
    float exp_left      = left.exposure;

    hdr.rgb       += (ldr_left) * w_left;
    weightSum     += w_left;

    // frame with higher exposure:
    vec3  ldr_right      = texture(right_exp, applyDisparity(resultUV)).rgb;
    float lum_right      = luminance(ldr_right);
    float w_right        = weight(lum_right);
    float exp_right      = right.exposure;

    hdr.rgb       += (ldr_right) * w_right;
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
	
	//cols *= left.exposure;
	cols.a = 1;

    //cols = pow(cols, vec4(1.0 / 2.2)); // gamma correction

    color = cols;
}