#pragma once

//GL
#include <GL/glew.h>

//FRAMEWORK
#include "glsl_program.h"
#include "texture.h"

//OTHER
#include <boost/optional.hpp>

//CPP
#include <memory>

namespace e2e
{
    class Quad
    {
    public:
        Quad();
        ~Quad();

        void create();
        void draw() const;
        void set_program(const GLSLProgram& program);
		void set_texture(std::shared_ptr<Texture> texture);
        void set_position(float x, float y);
        void set_scale_factor(float x, float y);
		void set_exposure(float x);

    private:
        GLuint m_vertex_array;
        GLuint m_vertex_buffer;

		GLfloat m_exposure;

		boost::optional<GLSLProgram> m_program;
		std::shared_ptr<Texture> m_texture;
        GLfloat m_position_x;
        GLfloat m_position_y;
        GLfloat m_scale_factor_x;
        GLfloat m_scale_factor_y;
    };
}