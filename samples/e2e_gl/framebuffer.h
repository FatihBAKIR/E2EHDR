#pragma once

//GL
#include <GL/glew.h>

//OTHER
#include <boost/core/noncopyable.hpp>

class Texture;

namespace e2e
{
	class Framebuffer : public boost::noncopyable
	{
	public:
		Framebuffer();
		~Framebuffer();

		void renderToTexture(const Texture & rtt_tex);
		void renderToTextureLayer(const Texture & rtt_tex, int layer);

	private:
		GLuint m_framebuffer_id;
	};
}