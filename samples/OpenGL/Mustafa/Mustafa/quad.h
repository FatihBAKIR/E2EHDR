#pragma once

//GL
#include <GL/glew.h>

//Forward decl.
class Pipeline;

class Quad
{
public:
	Quad();
	~Quad();

	void create();
	void addTexture(unsigned char* image, int width, int height);
	void draw(const Pipeline& pipeline) const;

private:
	GLuint m_vertex_array;
	GLuint m_vertex_buffer;
	GLuint m_texture;
};