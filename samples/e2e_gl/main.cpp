//FRAMEWORK
#include "glsl_program.h"
#include "quad.h"
#include "registration.h"
#include "Window.h"

using namespace e2e;

int main()
{
    e2e::Window w(800, 600);

	e2e::GLSLProgram hdr;
	hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
	hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/hdr.frag");
    hdr.link();

	e2e::Quad quad;
	quad.create();
    unsigned char image[] = {
            255,   0,   0,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255
    };
	Texture tex1;
	tex1.load(image, 3, 3);
	quad.set_texture(tex1);
	quad.set_program(hdr);
    quad.set_position(0.5f, 0.0f);
    quad.set_scale_factor(0.5f, 1.0f);

    e2e::Quad quad2;
    quad2.create();
    unsigned char image2[] = {
            255, 255, 255,
            255, 255, 255,
            255, 44, 255,
            255, 255, 128,
            255, 255, 255,
            255, 255, 255,
            255,   0, 255,
            255, 255, 255,
            255, 255, 255
    };
	Texture tex2;
	tex2.load(image2, 3, 3);
	quad2.set_texture(tex2);
	quad2.set_program(hdr);
	quad2.set_position(-0.5f, 0.0f);
	quad2.set_scale_factor(0.5f, 1.0f);

	while (!w.ShouldClose())
	{
        w.Loop({quad, quad2});
	}

	return 0;
}
