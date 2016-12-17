//FRAMEWORK
#include "merger.h"
#include "glsl_program.h"

//CPP
#include <assert.h>
#include <iostream>

#define __USE__MEDIAN__FILTER__

namespace e2e
{
	Merger::Merger(int image_width, int image_height, int disparity_limit)
		: m_image_width(image_width)
		, m_image_height(image_height)
		, m_disparity_limit(disparity_limit)
		, m_vertex_array(0)
		, m_vertex_buffer(0)
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

		//TEXTURES
		m_cost_texture.createArray(image_width, image_height, disparity_limit, nullptr);
		m_median_texture.create(image_width, image_height, nullptr);

		//SHADERS
		m_cost_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_cost_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/cost_adcensus.frag");
		m_cost_shader.link();

		m_aggregate_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_aggregate_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/aggregate_cross.frag");
		m_aggregate_shader.link();

		m_median_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_median_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/median_filter.frag");
		m_median_shader.link();
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
	}

	void Merger::draw()
	{
		//RESET
		set_position(0.0f, 0.0f);
		set_scale_factor(1.0f, 1.0f);

		//COST COMPUTATION//
		//MULTIPASS TO ARRAY TEXTURE
		m_cost_shader.use();
		static float dx = 1.0f / m_image_width;
		static float dy = 1.0f / m_image_height;
		m_cost_shader.setUniformFVar("dx", { dx });
		m_cost_shader.setUniformFVar("dy", { dy });
		for (GLint i = 0; i < m_disparity_limit; ++i)
		{
			m_cost_shader.setUniformIVar("disparity_level", { i });
			m_framebuffer.renderToTextureLayer(m_cost_texture, i);
			m_cost_texture.useArray();
			//glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_array_texture, 0, i);

			//Draw quad
			render(m_cost_shader);
		}

		//COST AGGREGATION//
#ifdef __USE__MEDIAN__FILTER__
		m_framebuffer.renderToTexture(m_median_texture);;
#else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
		m_aggregate_shader.use();
		m_aggregate_shader.setUniformIVar("disparity_limit", { m_disparity_limit });
		m_aggregate_shader.setUniformFVar("dx", { dx });
		m_aggregate_shader.setUniformFVar("dy", { dy });
		glActiveTexture(GL_TEXTURE2);
		m_aggregate_shader.setUniformIVar("dsi", { 2 });
		m_cost_texture.useArray();

		//Draw quad
		render(m_aggregate_shader);

#ifdef __USE__MEDIAN__FILTER__
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_median_shader.use();
		m_median_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_median_shader.setUniformFVar("translate", { m_position_x, m_position_y });
		m_median_shader.setUniformFVar("dx", { dx });
		m_median_shader.setUniformFVar("dy", { dy });

		glBindVertexArray(m_vertex_array);
		m_median_shader.setUniformIVar("prefiltered", { 0 });
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_median_texture.get_texture_id());

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
#endif
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