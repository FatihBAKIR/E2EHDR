#pragma once

//GL
#include "include\glad\glad.h"
#include "Window.h"

struct GLFWwindow;
struct ImDrawData;

namespace e2e
{
	class Window;

	void renderDrawList(ImDrawData* draw_data);
	const char* getClipboardText(void* user_data);
	void setClipboardText(void* user_data, const char* text);
	void mouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/);
	void scrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset);
	void keyCallback(GLFWwindow*, int key, int, int action, int mods);
	void charCallback(GLFWwindow*, unsigned int c);
	
	class GUI
	{
	public:
		GUI(const GUI&) = delete;
		GUI(GUI&&) = delete;
		GUI& operator=(const GUI&) = delete;
		GUI& operator=(GUI&&) = delete;

		static GUI& getGUI();

		bool initialize(e2e::Window& window, bool install_callbacks);
		void newFrame();

		friend void renderDrawList(ImDrawData* draw_data);
		friend const char* getClipboardText(void* user_data);
		friend void setClipboardText(void* user_data, const char* text);
		friend void mouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/);
		friend void scrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset);
		friend void keyCallback(GLFWwindow*, int key, int, int action, int mods);
		friend void charCallback(GLFWwindow*, unsigned int c);

	private:
		GLFWwindow*  m_window;
		double       m_time;
		bool         m_mouse_pressed[3];
		float        m_mouse_wheel;
		GLuint       m_font_texture;
		int          m_shader_handle, m_vert_handle, m_frag_handle;
		int          m_attrib_location_tex, m_attrib_location_proj_mtx;
		int          m_attrib_location_position, m_attrib_location_uv, m_attrib_location_color;
		unsigned int m_vbo_handle, m_vao_handle, m_elements_handle;

	private:
		GUI();
		~GUI();

		bool createFontsTexture();
		bool createDeviceObjects();
		void destroyDeviceObjects();
	};
}