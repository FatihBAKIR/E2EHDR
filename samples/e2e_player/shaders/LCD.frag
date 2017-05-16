#version 330

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution)
{
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.411764705882353);
    vec2 off2 = vec2(3.2941176470588234);
    vec2 off3 = vec2(5.176470588235294);

    vec4 coeff = vec4(0.2989, 0.5890, 0.114, 0.0);
    vec4 a = sqrt(vec4(vec3(dot(coeff, texture(image, uv))), 1.0));
    vec4 b = sqrt(vec4(vec3(dot(coeff, texture(image, uv + (off1 / resolution)))), 1.0));
    vec4 c = sqrt(vec4(vec3(dot(coeff, texture(image, uv - (off1 / resolution)))), 1.0));
    vec4 d = sqrt(vec4(vec3(dot(coeff, texture(image, uv + (off2 / resolution)))), 1.0));
    vec4 e = sqrt(vec4(vec3(dot(coeff, texture(image, uv - (off2 / resolution)))), 1.0));
    vec4 f = sqrt(vec4(vec3(dot(coeff, texture(image, uv + (off3 / resolution)))), 1.0));
    vec4 g = sqrt(vec4(vec3(dot(coeff, texture(image, uv - (off3 / resolution)))), 1.0));

    color += a * 0.1964825501511404;
    color += b * 0.2969069646728344;
    color += c * 0.2969069646728344;
    color += d * 0.09447039785044732;
    color += e * 0.09447039785044732;
    color += f * 0.010381362401148057;
    color += g * 0.010381362401148057;

    return color;
}

void main()
{
	vec4 clr = texture(texture0, tex_coord);

	clr *= pow(2.0, -3.0f);

    vec2 uv = vec2(gl_FragCoord.xy / vec2(2000, 1312));
    float asd = blur13(texture0, tex_coord, vec2(2000, 1312)).r;

	color = vec4(clr.r / asd, clr.g / asd, clr.b / asd, 1.0);
}