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
		, m_texture1(nullptr)
		, m_texture2(nullptr)
		, m_position_x(0.0f)
		, m_position_y(0.0f)
		, m_scale_factor_x(1.0f)
		, m_scale_factor_y(1.0f)
		, m_cost_choice(1)
		, m_aggregation_choice(0)
		, m_outlier_detection(false)
		, m_threshold(1.1f)
		, m_window_size(7)
		, m_outlier_correction(false)
		, m_median_filter(false)
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
		m_refinement_texture.create(image_width, image_height, nullptr);
		m_left_texture.create(image_width, image_height, nullptr);
		m_right_texture.create(image_width, image_height, nullptr);

		//SHADERS
		compileShaders();
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
		//SAVE THE DEFAULT VALUES
		float transformation[4] = { m_position_x, m_position_y, m_scale_factor_x, m_scale_factor_y };
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		//RESET
		set_position(0.0f, 0.0f);
		set_scale_factor(1.0f, 1.0f);
		static float dx = 1.0f / m_image_width;
		static float dy = 1.0f / m_image_height;

		//RECTIFICATION//
		m_framebuffer.renderToTexture(m_left_texture);
		m_undistort_left_shader.use();
		m_undistort_left_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_undistort_left_shader.setUniformFVar("translate", { m_position_x, m_position_y });
		glActiveTexture(GL_TEXTURE0);
		m_undistort_left_shader.setUniformIVar("frame", { 0 });
		m_texture1->use();
		render();

		m_framebuffer.renderToTexture(m_right_texture);
		m_undistort_right_shader.use();
		m_undistort_right_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_undistort_right_shader.setUniformFVar("translate", { m_position_x, m_position_y });
		glActiveTexture(GL_TEXTURE0);
		m_undistort_right_shader.setUniformIVar("frame", { 0 });
		m_texture2->use();
		render();

		//COST COMPUTATION//
		//MULTIPASS TO ARRAY TEXTURE
		m_cost_shader.use();
		m_cost_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_cost_shader.setUniformFVar("translate", { m_position_x, m_position_y });
		m_cost_shader.setUniformFVar("dx", { dx });
		m_cost_shader.setUniformFVar("dy", { dy });
		for (GLint i = 0; i < m_disparity_limit; ++i)
		{
			m_cost_shader.setUniformIVar("disparity_level", { i });
			m_framebuffer.renderToTextureLayer(m_cost_texture, i);

			glActiveTexture(GL_TEXTURE0);
			m_cost_shader.setUniformIVar("left", { 0 });
			m_left_texture.use();
			glActiveTexture(GL_TEXTURE1);
			m_cost_shader.setUniformIVar("right", { 1 });
			m_right_texture.use();
			glActiveTexture(GL_TEXTURE2);
			m_cost_shader.setUniformIVar("dsi", { 2 });
			m_cost_texture.useArray();

			//Draw quad
			render();
		}

		//COST AGGREGATION//
		m_framebuffer.renderToTexture(m_refinement_texture);
		m_aggregate_shader.use();
		m_aggregate_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_aggregate_shader.setUniformFVar("translate", { m_position_x, m_position_y });
		m_aggregate_shader.setUniformIVar("disparity_limit", { m_disparity_limit });
		m_aggregate_shader.setUniformFVar("dx", { dx });
		m_aggregate_shader.setUniformFVar("dy", { dy });

		glActiveTexture(GL_TEXTURE0);
		m_aggregate_shader.setUniformIVar("left", { 0 });
		m_left_texture.use();
		glActiveTexture(GL_TEXTURE1);
		m_aggregate_shader.setUniformIVar("right", { 1 });
		m_right_texture.use();
		glActiveTexture(GL_TEXTURE2);
		m_aggregate_shader.setUniformIVar("dsi", { 2 });
		m_cost_texture.useArray();

		//Draw quad
		render();

		//OUTLIER DETECTION//
		//
		if (m_outlier_detection)
		{
			m_framebuffer.renderToTexture(m_refinement_texture);
			m_outlier_detection_shader.use();
			m_outlier_detection_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
			m_outlier_detection_shader.setUniformFVar("translate", { m_position_x, m_position_y });
			m_outlier_detection_shader.setUniformIVar("N", { m_window_size / 2 });
			m_outlier_detection_shader.setUniformFVar("threshold", { m_threshold });
			m_outlier_detection_shader.setUniformFVar("dx", { dx });
			m_outlier_detection_shader.setUniformFVar("dy", { dy });

			glActiveTexture(GL_TEXTURE0);
			m_outlier_detection_shader.setUniformIVar("disparity_map", { 0 });
			m_refinement_texture.use();
			glActiveTexture(GL_TEXTURE1);
			m_outlier_detection_shader.setUniformIVar("dsi", { 1 });
			m_cost_texture.useArray();

			//Draw quad
			render();
		}

		//OUTLIER CORRECTION//
		//
		if (m_outlier_detection && m_outlier_correction)
		{
			m_framebuffer.renderToTexture(m_refinement_texture);
			m_outlier_correction_shader.use();
			m_outlier_correction_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
			m_outlier_correction_shader.setUniformFVar("translate", { m_position_x, m_position_y });
			m_outlier_correction_shader.setUniformFVar("dx", { dx });
			m_outlier_correction_shader.setUniformFVar("dy", { dy });

			glActiveTexture(GL_TEXTURE0);
			m_outlier_correction_shader.setUniformIVar("disparity_map", { 0 });
			m_refinement_texture.use();

			//Draw quad
			render();
		}

		//MEDIAN FILTER//
		//
		if (m_median_filter)
		{
			m_framebuffer.renderToTexture(m_refinement_texture);
			m_median_shader.use();
			m_median_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
			m_median_shader.setUniformFVar("translate", { m_position_x, m_position_y });
			m_median_shader.setUniformFVar("dx", { dx });
			m_median_shader.setUniformFVar("dy", { dy });

			glActiveTexture(GL_TEXTURE0);
			m_median_shader.setUniformIVar("disparity_map", { 0 });
			m_refinement_texture.use();

			//Draw quad
			render();
		}

		//BACK TO DEFAULT VALUES
		set_position(transformation[0], transformation[1]);
		set_scale_factor(transformation[2], transformation[3]);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_hdr_merge_shader.use();
		m_hdr_merge_shader.setUniformFVar("scale", { m_scale_factor_x, m_scale_factor_y });
		m_hdr_merge_shader.setUniformFVar("translate", { m_position_x, m_position_y });
		m_hdr_merge_shader.setUniformFVar("dx", { dx });
		m_hdr_merge_shader.setUniformFVar("dy", { dy });

		glActiveTexture(GL_TEXTURE0);
		m_hdr_merge_shader.setUniformIVar("disparity_map", { 0 });
		m_refinement_texture.use();

		//Draw quad
		render();
	}

	void Merger::compileShaders()
	{
		m_undistort_left_shader.clear();
		m_undistort_left_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_undistort_left_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/undistort.frag");
		m_undistort_left_shader.link();

		m_undistort_right_shader.clear();
		m_undistort_right_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_undistort_right_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/undistort.frag");
		m_undistort_right_shader.link();

		int selection = m_cost_choice;
		m_cost_choice = -1;
		chooseCost(selection);
		selection = m_aggregation_choice;
		m_aggregation_choice = -1;
		chooseAggregation(selection);

		m_outlier_detection_shader.clear();
		m_outlier_detection_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_outlier_detection_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/outlier_detection.frag");
		m_outlier_detection_shader.link();

		m_outlier_correction_shader.clear();
		m_outlier_correction_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_outlier_correction_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/outlier_correction.frag");
		m_outlier_correction_shader.link();

		m_median_shader.clear();
		m_median_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_median_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/median_filter.frag");
		m_median_shader.link();

		m_hdr_merge_shader.clear();
		m_hdr_merge_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
		m_hdr_merge_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/basic.frag");
		m_hdr_merge_shader.link();
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

	GLSLProgram & Merger::get_cost_shader()
	{
		return m_cost_shader;
	}

	GLSLProgram & Merger::get_undistort_left_shader()
	{
		return m_undistort_left_shader;
	}

	GLSLProgram & Merger::get_undistort_right_shader()
	{
		return m_undistort_right_shader;
	}

	void Merger::chooseCost(int selection)
	{
		if (selection != m_cost_choice)
		{
			std::string vert_path = "shaders/hdr.vert"; 
			std::string frag_path;
			switch (selection)
			{
			case 0:
			{
				frag_path = "shaders/cost_ad.frag";
			}
			break;

			case 1:
			{
				frag_path = "shaders/cost_adcensus.frag";
			}
			break;

			case 2:
			{
				frag_path = "shaders/cost_census.frag";
			}
			break;

			case 3:
			{
				frag_path = "shaders/cost_census_modified.frag";
			}
			break;
			}

			m_cost_shader.clear();
			m_cost_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, vert_path.c_str());
			m_cost_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, frag_path.c_str());
			m_cost_shader.link();
			m_cost_choice = selection;
		}
	}

	void Merger::chooseAggregation(int selection)
	{
		if (selection != m_aggregation_choice)
		{
			std::string vert_path = "shaders/hdr.vert";
			std::string frag_path;
			switch (selection)
			{
			case 0:
			{
				frag_path = m_outlier_detection ? "shaders/aggregate3x3withAPKR.frag" : "shaders/aggregate3x3.frag";
			}
			break;

			case 1:
			{
				frag_path = m_outlier_detection ? "shaders/aggregate_crosswithAPKR.frag" : "shaders/aggregate_cross.frag";
			}
			break;
			}

			m_aggregate_shader.clear();
			m_aggregate_shader.attachShader(e2e::GLSLProgram::VERTEX_SHADER, vert_path.c_str());
			m_aggregate_shader.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, frag_path.c_str());
			m_aggregate_shader.link();
			m_aggregation_choice = selection;
		}
	}

	void Merger::set_outlier_detection(bool outlier_detection, float threshold, int window_size)
	{
		if (m_outlier_detection != outlier_detection)
		{
			m_outlier_detection = outlier_detection;
			int selection = m_aggregation_choice;
			m_aggregation_choice = -1;
			chooseAggregation(selection);
		}

		m_threshold = threshold;
		m_window_size = window_size;
	}

	void Merger::set_outlier_correction(bool outlier_correction)
	{
		m_outlier_correction = outlier_correction;
	}

	void Merger::set_median_filter(bool median_filter)
	{
		m_median_filter = median_filter;
	}

	void Merger::render()
	{
		glBindVertexArray(m_vertex_array);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}