#version 330

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;
uniform vec2 im_scale;
uniform int ldr_mode;

float inv_lerp(float edge0, float edge1, float x)
{
    return clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
}

void main()
{
    if (ldr_mode == 1)
    {
        color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    float u = inv_lerp(0.5 - im_scale.x / 2, 0.5 + im_scale.x / 2, tex_coord.x);
    float v = inv_lerp(0.5 - im_scale.y / 2, 0.5 + im_scale.y / 2, tex_coord.y);

    vec2 x = vec2(u, v);

    if (x.x == 0 || x.x == 1 || x.y == 0 || x.y == 1)
    {
        color = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

	vec4 clr = texture(texture0, x);

	clr *= pow(2.0, -3.0f);

	vec4 coeff = vec4(0.2989, 0.5890, 0.114, 0.0);
	float col = dot(coeff, clr);
	col = sqrt(col);

	color = vec4(vec3(col), 1.0);
}
