//FRAMEWORK
#include "quad.h"
#include "pipeline.h"

namespace e2e
{
Quad::Quad()
	: m_vertex_array(0)
	, m_vertex_buffer(0)
	, m_texture(0)
{}

Quad::~Quad()
{
	if (m_texture)
	{
		glDeleteTextures(1, &m_texture);
	}

	if (m_vertex_buffer)
	{
		glDeleteBuffers(1, &m_vertex_buffer);
	}

	if (m_vertex_array)
	{
		glDeleteVertexArrays(1, &m_vertex_array);
	}
}

void Quad::create()
{
	GLfloat vertices[] = {
		//NDC coordinates for the quad.
		//Positions     //Texture coordinates
		-1.0f,  1.0f,   0.0f, 1.0f,
		-1.0f, -1.0f,   0.0f, 0.0f,
		1.0f , -1.0f,   1.0f, 0.0f,

		-1.0f,  1.0f,   0.0f, 1.0f,
		1.0f , -1.0f,   1.0f, 0.0f,
		1.0f ,  1.0f,   1.0f, 1.0f
	};

	glGenVertexArrays(1, &m_vertex_array);
	glGenBuffers(1, &m_vertex_buffer);
	glBindVertexArray(m_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);
}

void Quad::addTexture(unsigned char* image, int width, int height)
{
	if (m_texture)
	{
		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_texture);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);

	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Quad::draw() const
{
    if (!material) return;

	material->use();
	glBindVertexArray(m_vertex_array);

    glUniform2f(glGetUniformLocation(material->get_pipeline(), "scale"), m_scale_factor_x, m_scale_factor_y);
    glUniform2f(glGetUniformLocation(material->get_pipeline(), "translate"), m_position_x, m_position_y);

	if (m_texture)
	{
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(material->get_pipeline(), "texture0"), 0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Quad::set_material(Material &mat)
{
	material = mat;
}

void Quad::set_position(float x, float y)
{
    m_position_x = x;
    m_position_y = y;
}

void Quad::set_scale_factor(float x, float y)
{
    m_scale_factor_x = x;
    m_scale_factor_y = y;
}
}