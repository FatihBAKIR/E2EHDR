//FRAMEWORK
#include "framebuffer.h"
#include "texture.h"

//CPP
#include <assert.h>
#include <iostream>

namespace e2e
{
	Framebuffer::Framebuffer()
	{
		glGenFramebuffers(1, &m_framebuffer_id);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			throw std::runtime_error("ERROR::Framebuffer is not complete!");
		}
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_framebuffer_id);
	}

	void Framebuffer::renderToTexture(const Texture& rtt_tex)
	{
		glViewport(0, 0, rtt_tex.get_width(), rtt_tex.get_height());
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rtt_tex.get_texture_id(), 0);
	}

    void Framebuffer::renderToTexture2D(const Texture& rtt_tex1, const Texture& rtt_tex2)
    {
        glViewport(0, 0, rtt_tex1.get_width(), rtt_tex1.get_height());
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtt_tex1.get_texture_id(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, rtt_tex2.get_texture_id(), 0);
        GLenum drawbuf[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawbuf);
    }

    void Framebuffer::renderToTexture2D(const Texture& rtt_tex1, const Texture& rtt_tex2, const Texture& rtt_tex3)
    {
        glViewport(0, 0, rtt_tex1.get_width(), rtt_tex1.get_height());
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtt_tex1.get_texture_id(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, rtt_tex2.get_texture_id(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, rtt_tex3.get_texture_id(), 0);
        GLenum drawbuf[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, drawbuf);
    }

	void Framebuffer::renderToTextureLayer(const Texture & rtt_tex, int layer)
	{
		glViewport(0, 0, rtt_tex.get_width(), rtt_tex.get_height());
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rtt_tex.get_texture_id(), 0, layer);
	}
}