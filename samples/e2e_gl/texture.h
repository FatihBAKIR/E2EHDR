#pragma once

//GL
#include <GL/glew.h>

//OTHER
#include <boost/core/noncopyable.hpp>

class Texture : public boost::noncopyable
{
public:
	Texture();
	~Texture();

	void load(unsigned char* image, int width, int height);
	void use() const;

private:
	GLuint m_id;
};