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
    quad.addTexture(image, 3, 3);
    quad.set_position(-0.51, 0);
    quad.set_scale_factor(0.49, 1);
    quad.set_material(hdr);

    e2e::Quad quad2;
    quad2.create();
    unsigned char image2[] = {
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255, 255, 255,
            255,   0, 255,
            255, 255, 255,
            255, 255, 255
    };
    quad2.addTexture(image2, 3, 3);
    quad2.set_position(0.5, 0);
    quad2.set_scale_factor(0.5, 1);
    quad2.set_material(hdr);

	while (!w.ShouldClose())
	{
        w.Loop({quad, quad2});
	}

	return 0;
}
