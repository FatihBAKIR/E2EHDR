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

	void create(int width, int height, unsigned char* image);
	void createArray(int width, int height, int layer, unsigned char* image);
	void use() const;
	void useArray() const;

	GLuint get_texture_id() const { return m_texture_id; }
	GLuint get_width() const { return m_width; }
	GLuint get_height() const { return m_height; }

private:
	GLuint m_texture_id;
	int m_width;
	int m_height;
};