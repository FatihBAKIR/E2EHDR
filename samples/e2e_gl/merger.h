#pragma once

//GL
#include <GL/glew.h>

//FRAMEWORK
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
		void set_textures(const Texture& left, const Texture& right);
		void set_position(float x, float y);
		void set_scale_factor(float x, float y);

	private:
		int m_image_width;
		int m_image_height;
		int m_disparity_limit;

		GLuint m_vertex_array;
		GLuint m_vertex_buffer;
		GLuint m_framebuffer;
		GLuint m_array_texture;
		const Texture* m_texture1;
		const Texture* m_texture2;

		GLSLProgram m_cost;
		GLSLProgram m_aggregate;

		GLfloat m_position_x;
		GLfloat m_position_y;
		GLfloat m_scale_factor_x;
		GLfloat m_scale_factor_y;

	private:
		void render(const GLSLProgram& program);
	};
}