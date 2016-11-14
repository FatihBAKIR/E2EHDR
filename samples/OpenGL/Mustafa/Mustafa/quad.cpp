//FRAMEWORK
#include "quad.h"
#include "glsl_program.h"

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

	glGenTextures(1, &m_texture);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Quad::draw(const GLSLProgram& program) const
{
	glBindVertexArray(m_vertex_array);

	if (m_texture)
	{
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(program.get_pipeline(), "texture0"), 0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}