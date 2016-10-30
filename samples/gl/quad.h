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

    private:
        boost::optional<Material> material;
        GLuint m_vertex_array;
        GLuint m_vertex_buffer;
        GLuint m_texture;
    };
}
