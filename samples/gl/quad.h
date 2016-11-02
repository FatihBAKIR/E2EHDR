#pragma once

//GL
#include <GL/glew.h>
#include <boost/optional.hpp>
#include "pipeline.h"

namespace e2e
{
    class Quad
    {
    public:
        Quad();
        ~Quad();

        void create();
        void addTexture(unsigned char* image, int width, int height);
        void draw() const;
        void set_material(Material& mat);
        void set_position(float x, float y);
        void set_scale_factor(float x, float y);

    private:
        boost::optional<Material> material;
        GLuint m_vertex_array;
        GLuint m_vertex_buffer;
        GLuint m_texture;

        GLfloat m_position_x;
        GLfloat m_position_y;
        GLfloat m_scale_factor_x;
        GLfloat m_scale_factor_y;
    };
}
