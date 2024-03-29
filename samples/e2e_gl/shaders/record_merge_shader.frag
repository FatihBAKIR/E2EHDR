#version 430 core

in vec2 tex_coord;
out vec4 color;

uniform float dx;
uniform float dy;
uniform sampler2D disparity_map;
uniform sampler2D left_exp;
uniform sampler2D right_exp;
uniform int color_debug;

struct camera_params
{
    float exposure;
};

uniform camera_params left;
uniform camera_params right;

vec2 applyDisparity(vec2 point)
{
	float  offset = texture(disparity_map, vec2(point.x, 1 - point.y)).r * 31;
    return vec2(point.x, point.y + offset*dy);
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

	//If it is well exposed then do not consider
	//other frame
	if (w_left < 0.98f)
	{
	    if (color_debug==0)
		{
			return vec4(ldr_left, 1.0f);
		}
		else
		{
			return vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
	}

    // frame with higher exposure:
    vec4 sample_right      = texture(right_exp, applyDisparity(resultUV));
    vec3  ldr_right      = sample_right.rgb;
    float w_right        = sample_right.a;

	if (color_debug==0)
	{
        hdr.rgb       += (ldr_right) * w_right;
        weightSum     += w_right;

        // overall value:
        hdr.rgb /= weightSum + 1e-6;
        hdr.a    = 1;

        return hdr; // returns the rgba value.
	}
	else
	{
		return vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
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
	color = createHDR(resultUV);
}
