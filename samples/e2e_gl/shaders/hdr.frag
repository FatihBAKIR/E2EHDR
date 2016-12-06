#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

uniform float crf_r[256];
uniform float crf_g[256];
uniform float crf_b[256];

uniform float exposure;

void main()
{
    //-15,5
    float exposure = -2;
    float gamma = 0.1;

    vec4 col = texture(texture0, vec2(tex_coord.x, 1-tex_coord.y));

	float r = crf_r[min(int(col.r * 255), 255)];
	float g = crf_g[min(int(col.g * 255), 255)];
	float b = crf_b[min(int(col.b * 255), 255)];

    vec3 cols = vec3(r, g, b);

    //cols /= exposure;

    //cols = cols * pow(2.0, exposure);
	//cols = pow(cols, vec3(pow(2.0, gamma)));
	cols *= 8;
	cols = pow(cols, vec3(1.0 / 2.2)); // gamma correction
    color = vec4(cols, 1.0);

    //color = col;
}