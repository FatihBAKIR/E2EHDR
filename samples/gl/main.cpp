//FRAMEWORK
#include "pipeline.h"
#include "quad.h"
#include "Window.h"

int main()
{
    e2e::Window w(800, 600);

	Material hdr;
	hdr.attachShader(Material::VERTEX_SHADER, "shaders/hdr.vert");
	hdr.attachShader(Material::FRAGMENT_SHADER, "shaders/hdr.frag");
    hdr.link();

	e2e::Quad quad;
	quad.create();
    unsigned char image[] = {
            255, 0, 0,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255
    };
	quad.addTexture(image, 3, 3);
    quad.set_material(hdr);

	while (!w.ShouldClose())
	{
        w.Loop(quad);
	}

	return 0;
}
