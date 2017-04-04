//FRAMEWORK
#include "texture.h"

//CPP
#include <assert.h>

Texture::Texture()
	: m_texture_id(0)
{}

Texture::~Texture()
{
	if (m_texture_id)
	{
		glDeleteTextures(1, &m_texture_id);
	}
}

Texture::Texture(Texture&& texture)
{
    m_texture_id = texture.m_texture_id;
    m_width = texture.m_width;
    m_height = texture.m_height;
    texture.m_texture_id = 0;
}

Texture& Texture::operator=(Texture&& texture)
{
    m_texture_id = texture.m_texture_id;
    m_width = texture.m_width;
    m_height = texture.m_height;
    texture.m_texture_id = 0;

    return *this;
}

void Texture::create(int width, int height, unsigned char* image)
{
	if (m_texture_id)
	{
		glDeleteTextures(1, &m_texture_id);
		m_texture_id = 0;
	}

	m_width = width;
	m_height = height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_texture_id);

	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::createArray(int width, int height, int layer, unsigned char* image)
{
	if (m_texture_id)
	{
		glDeleteTextures(1, &m_texture_id);
		m_texture_id = 0;
	}

	m_width = width;
	m_height = height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_texture_id);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_id);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, layer, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	//Settings
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture::createFloat(int width, int height, float* data)
{
	if (m_texture_id)
	{
		glDeleteTextures(1, &m_texture_id);
		m_texture_id = 0;
	}

	m_width = width;
	m_height = height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_texture_id);

	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);

	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::use() const
{
	assert(m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
}

void Texture::useArray() const
{
	assert(m_texture_id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_id);
}

void* Texture::getTextureImage()
{
    auto size = 3 * m_width * m_height * sizeof(unsigned char);
    void* pixels = new unsigned char[size];
    glGetTextureImage(m_texture_id, 0, GL_RGB, GL_UNSIGNED_BYTE, size, pixels);

    return pixels;
}

void Texture::create_mipmaps() const
{
    use();
	glGenerateTextureMipmap(m_texture_id);
}
