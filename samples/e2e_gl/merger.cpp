//FRAMEWORK
#include "merger.h"
#include "glsl_program.h"

//CPP
#include <assert.h>
#include <iostream>

namespace e2e
{
	Merger::Merger(int image_width, int image_height, int disparity_limit)
		: m_image_width(image_width)
		, m_image_height(image_height)
		, m_disparity_limit(disparity_limit)
		, m_vertex_array(0)
		, m_vertex_buffer(0)
		, m_framebuffer(0)
		, m_array_texture(0)
		, m_texture1(nullptr)
		, m_texture2(nullptr)
		, m_position_x(0.0f)
		, m_position_y(0.0f)
		, m_scale_factor_x(1.0f)
		, m_scale_factor_y(1.0f)
	{
		GLfloat vertices[] =
		{
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

		//FRAMEBUFFER
		glGenFramebuffers(1, &m_framebuffer);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			throw std::runtime_error("ERROR::MERGER::Framebuffer is not complete!");
		}

		//ARRAY TEXTURE
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &m_array_texture);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_array_texture);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, image_width, image_height, disparity_limit, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		//SHADERS
		m_cost.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_cost.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/cost_adcensus.frag");
		m_cost.link();

		m_aggregate.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_aggregate.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/aggregate3x3.frag");
		m_aggregate.link();
	}

	Merger::~Merger()
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

		if (m_framebuffer)
		{
			glDeleteFramebuffers(1, &m_framebuffer);
		}

		if (m_array_texture)
		{
			glDeleteTextures(1, &m_array_texture);
		}
	}

	void Merger::draw()
	{
		//RESET
		set_position(0.0f, 0.0f);
		set_scale_factor(1.0f, 1.0f);

		//RENDER//
		//MULTIPASS TO ARRAY TEXTURE
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		glClear(GL_COLOR_BUFFER_BIT);

		m_cost.setUniformFVar("dx", { 1.0f / m_image_width });
		m_cost.setUniformFVar("dy", { 1.0f / m_image_height });
		for (GLint i = 0; i < m_disparity_limit; ++i)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_array_texture);
			m_cost.setUniformIVar("disparity_level", { i });
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_array_texture, 0, i);

			render(m_cost);
		}

		//SECOND PASS
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		m_aggregate.setUniformFVar("dx", { 1.0f / m_image_width });
		m_aggregate.setUniformFVar("dy", { 1.0f / m_image_height });
		glActiveTexture(GL_TEXTURE2);
		m_aggregate.setUniformIVar("dsi", { 2 });
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_array_texture);

		//Draw quad
		render(m_aggregate);
	}

	void Merger::set_textures(const Texture & left, const Texture & right)
	{
		m_texture1 = &left;
		m_texture2 = &right;
	}

	void Merger::set_position(float x, float y)
	{
		m_position_x = x;
		m_position_y = y;
	}

	void Merger::set_scale_factor(float x, float y)
	{
		m_scale_factor_x = x;
		m_scale_factor_y = y;
	}

	void Merger::render(const GLSLProgram& program)
	{
		program.use();
		program.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		program.setUniformFVar("translate", { m_position_x, m_position_y });

		glBindVertexArray(m_vertex_array);
		program.setUniformIVar("left", { 0 });
		glActiveTexture(GL_TEXTURE0);
		assert(m_texture1);
		m_texture1->use();
		glActiveTexture(GL_TEXTURE1);
		program.setUniformIVar("right", { 1 });
		assert(m_texture2);
		m_texture2->use();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
}