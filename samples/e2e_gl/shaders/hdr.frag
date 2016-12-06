#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

struct crf
{
    float red[256];
    float green[256];
    float blue[256];
};

uniform crf response;

uniform float exposure;

int get_byte(float channel)
{
    return min(int(channel * 255), 255);
}

vec3 apply_crf(vec3 col)
{
	float r = response.red[get_byte(col.r)];
	float g = response.green[get_byte(col.g)];
	float b = response.blue[get_byte(col.b)];

    return vec3(r, g, b);
}

void main()
{
    //-15,5
    float exposure = -2;
    float gamma = 0.1;

    vec4 col = texture(texture0, vec2(tex_coord.x, 1-tex_coord.y));

    vec3 cols = apply_crf(vec3(col));

    cols /= exposure;

    //cols = cols * pow(2.0, exposure);
	//cols = pow(cols, vec3(pow(2.0, gamma)));
	//cols *= 8;
	cols = pow(cols, vec3(1.0 / 2.2)); // gamma correction
    color = vec4(cols, 1.0);

    //color = col;
}