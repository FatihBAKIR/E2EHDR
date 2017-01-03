#pragma once

//GL
#include <GL/glew.h>

//FRAMEWORK
#include "framebuffer.h"
#include "glsl_program.h"
#include "texture.h"
#include "quad.h"

namespace e2e
{
	class Merger
	{
	public:
		Merger(int image_width, int image_height, int disparity_limit);
		~Merger();

		void draw();
		void compileShaders();

		void set_textures(const Texture& left, const Texture& right);
		void set_position(float x, float y);
		void set_scale_factor(float x, float y);
		GLSLProgram& get_cost_shader();
		GLSLProgram& get_undistort_left_shader();
		GLSLProgram& get_undistort_right_shader();
		GLSLProgram& get_merge_shader();

		//Parameter setters and functions
		void chooseCost(int selection);
		void chooseAggregation(int selection);
		void set_outlier_detection(bool outlier_detection, float threshold, int window_size);
		void set_outlier_correction(bool outlier_correction);
		void set_median_filter(bool median_filter);

	private:
		int m_window_width;
		int m_window_height;
		int m_image_width;
		int m_image_height;
		int m_disparity_limit;

		GLuint m_vertex_array;
		GLuint m_vertex_buffer;

		Framebuffer m_framebuffer;
		Texture m_cost_texture;
		Texture m_refinement_texture;
		Texture m_left_texture;
		Texture m_right_texture;
		const Texture* m_texture1;
		const Texture* m_texture2;

		GLSLProgram m_undistort_left_shader;
		GLSLProgram m_undistort_right_shader;
		GLSLProgram m_cost_shader;
		GLSLProgram m_aggregate_shader;
		GLSLProgram m_outlier_detection_shader;
		GLSLProgram m_outlier_correction_shader;
		GLSLProgram m_median_shader;
		GLSLProgram m_hdr_merge_shader;

		GLfloat m_position_x;
		GLfloat m_position_y;
		GLfloat m_scale_factor_x;
		GLfloat m_scale_factor_y;

		//Setting parameters.
		int m_cost_choice;
		int m_aggregation_choice;
		
		bool m_outlier_detection;
		float m_threshold;
		int m_window_size;
		bool m_outlier_correction;
		bool m_median_filter;

	private:
		void render();
	};
}