#version 330

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texture0;

void main()
{
	vec4 clr = texture(texture0, tex_coord);

	vec4 coeff = vec4(0.2989, 0.5890, 0.114, 0.0);
	float col = dot(coeff, clr);
	col = sqrt(col);

	color = vec4(vec3(col), 1.0);
}
