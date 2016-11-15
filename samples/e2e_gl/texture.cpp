//FRAMEWORK
#include "texture.h"

//CPP
#include <assert.h>

Texture::Texture()
	: m_id(0)
{}

Texture::~Texture()
{
	if (m_id)
	{
		glDeleteTextures(1, &m_id);
	}
}

void Texture::load(unsigned char* image, int width, int height)
{
	if (m_id)
	{
		glDeleteTextures(1, &m_id);
		m_id = 0;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_id);

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::use() const
{
	assert(m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
}