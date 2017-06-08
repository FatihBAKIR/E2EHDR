#version 330 core

in vec3 avg;
out vec4 color;

void main() {
	color = vec4(avg, 1.0);
}
