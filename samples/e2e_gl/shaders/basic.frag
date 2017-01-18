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
    float exposure;
};

uniform camera_params left;
uniform camera_params right;

vec2 applyDisparity(vec2 point) 
{
	float  offset = texture(disparity_map, vec2(point.x, 1 - point.y)).r * 31;
    return vec2(point.x - offset*dx, point.y);
}

vec4 createHDR(vec2 resultUV)
{
    float weightSum = 0.0;
    vec4 hdr = vec4(0.0, 0.0, 0.0, 1.0);

    // frame with lower exposure:
	vec4 sample_left = texture(left_exp, resultUV);
    vec3  ldr_left      = sample_left.rgb;
    float w_left        = sample_left.a;

    hdr.rgb       += (ldr_left) * w_left;
    weightSum     += w_left;

    // frame with higher exposure:
    vec4 sample_right      = texture(right_exp, applyDisparity(resultUV));
    vec3  ldr_right      = sample_right.rgb;
    float w_right        = sample_right.a;

    hdr.rgb       += (ldr_right) * w_right;
    weightSum     += w_right;

    // overall value:
    hdr.rgb /= weightSum + 1e-6;
    hdr.a    = 1;

    return hdr; // returns the rgba value.
}

uniform float base;
uniform float maxLum;

vec3 tonemap(vec3 color)
{
	color /= maxLum;
	color = log(color + vec3(1.0)) / log(base + 1.0);
	return color;
}

void main()
{
    vec2 resultUV = tex_coord;
    resultUV.y = 1 - resultUV.y;
	
    vec4 cols = createHDR(resultUV);
	
	//cols = vec4(tonemap(cols.rgb), 1.0);
	cols *= pow(2.0, base);
	
    cols = pow(cols, vec4(1.0 / 2.2)); // gamma correction
	cols.a = 1;
    color = cols;
}