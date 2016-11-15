//FRAMEWORK
#include "quad.h"
#include "glsl_program.h"

//CPP
#include <assert.h>

namespace e2e
{
	Quad::Quad()
		: m_vertex_array(0)
		, m_vertex_buffer(0)
		, m_texture(nullptr)
	{}

	Quad::~Quad()
	{
		if (m_vertex_buffer)
		{
			glDeleteBuffers(1, &m_vertex_buffer);
			m_vertex_buffer = 0;
		}

		if (m_vertex_array)
		{
			glDeleteVertexArrays(1, &m_vertex_array);
			m_vertex_array = 0;
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

	void Quad::draw() const
	{
		assert(m_program);
		m_program->use();
		m_program->setUniformIVar("texture0", { 0 });
		m_program->setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_program->setUniformFVar("translate", { m_position_x, m_position_y });

		glBindVertexArray(m_vertex_array);
		glActiveTexture(GL_TEXTURE0);
		assert(m_texture);
		m_texture->use();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

	void Quad::set_program(const GLSLProgram &program)
	{
		m_program = program;
	}

	void Quad::set_texture(std::shared_ptr<Texture> texture)
	{
		m_texture = texture;
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