#pragma once

//GL
#include "include/glad/glad.h"

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
        void renderToTexture2D(const Texture & rtt_tex1, const Texture & rtt_tex2);
        void renderToTexture2D(const Texture & rtt_tex1, const Texture & rtt_tex2, const Texture & rtt_tex3);
		void renderToTextureLayer(const Texture & rtt_tex, int layer);

	private:
		GLuint m_framebuffer_id;
	};
}