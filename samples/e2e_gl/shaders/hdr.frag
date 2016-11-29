#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;
uniform float crf_r[256];
uniform float crf_g[256];
uniform float crf_b[256];


void main()
{
    float exposure = -5;

    vec4 col = texture(texture0, vec2(tex_coord.x, 1-tex_coord.y));
	float r = crf_r[int(col.r * 255)];
	float g = crf_g[int(col.g * 255)];
	float b = crf_b[int(col.b * 255)];

    vec3 cols = vec3(r, g, b) * pow(2.0, exposure);
    color = vec4(pow(vec3(cols), vec3(0.4545)), 1.0); // gamma correction
}