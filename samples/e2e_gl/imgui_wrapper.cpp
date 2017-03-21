//FRAMEWORK
#include "imgui_wrapper.h"
#include "glsl_program.h"

//OTHER
#include <imgui.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

namespace e2e
{
	GUI& GUI::getGUI()
	{
		static GUI gui;
		return gui;
	}

	GUI::GUI()
		: m_window(nullptr)
		, m_mouse_wheel(0.0f)
		, m_font_texture(0)
		, m_shader_handle(0)
		, m_vert_handle(0)
		, m_frag_handle(0)
		, m_attrib_location_tex(0)
		, m_attrib_location_proj_mtx(0)
		, m_attrib_location_position(0)
		, m_attrib_location_uv(0)
		, m_attrib_location_color(0)
		, m_vbo_handle(0)
		, m_vao_handle(0)
		, m_elements_handle(0)
	{
		m_mouse_pressed[0] = false;
		m_mouse_pressed[1] = false;
		m_mouse_pressed[2] = false;
	}

	GUI::~GUI()
	{
		destroyDeviceObjects();
		ImGui::Shutdown();
	}

	bool GUI::initialize(e2e::Window& window, bool install_callbacks)
	{
		m_window = window.m_window_primary;

		//Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		io.RenderDrawListsFn = &renderDrawList;
		io.SetClipboardTextFn = &setClipboardText;
		io.GetClipboardTextFn = &getClipboardText;
		io.ClipboardUserData = m_window;
#ifdef _WIN32
		io.ImeWindowHandle = glfwGetWin32Window(m_window);
#endif

		if (install_callbacks)
		{
			glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
			glfwSetScrollCallback(m_window, scrollCallback);
			glfwSetKeyCallback(m_window, keyCallback);
			glfwSetCharCallback(m_window, charCallback);
		}

		return true;
	}

	void GUI::newFrame()
	{
		if (!m_font_texture)
		{
			createDeviceObjects();
		}

		ImGuiIO& io = ImGui::GetIO();

		//Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(m_window, &w, &h);
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
		io.DisplayFramebufferScale = ImVec2(w > 0 ? (static_cast<float>(display_w) / w) : 0, h > 0 ? (static_cast<float>(display_h) / h) : 0);

		//Setup time step
		double current_time = glfwGetTime();
		io.DeltaTime = m_time > 0.0 ? static_cast<float>(current_time - m_time) : static_cast<float>(1.0f / 60.0f);
		m_time = current_time;

		//Setup inputs
		if (glfwGetWindowAttrib(m_window, GLFW_FOCUSED))
		{
			double mouse_x, mouse_y;
			glfwGetCursorPos(m_window, &mouse_x, &mouse_y);
			io.MousePos = ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
		}
		else
		{
			io.MousePos = ImVec2(-1, -1);
		}

		for (int i = 0; i < 3; ++i)
		{
			//If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
			io.MouseDown[i] = m_mouse_pressed[i] || glfwGetMouseButton(m_window, i) != 0;

			m_mouse_pressed[i] = false;
		}

		io.MouseWheel = m_mouse_wheel;
		m_mouse_wheel = 0.0f;

		//Hide OS mouse cursor if ImGui is drawing it
		glfwSetInputMode(m_window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

		//Start the frame
		ImGui::NewFrame();
	}

	void renderDrawList(ImDrawData* draw_data)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		int fb_width = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
		{
			return;
		}
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		//Backup GL state.
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		GLint last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
		GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
		GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
		GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
		GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
		GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
		GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

		//Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled.
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glActiveTexture(GL_TEXTURE0);

		//Setup viewport, orthographic projection matrix.
		glViewport(0, 0, static_cast<GLsizei>(fb_width), static_cast<GLsizei>(fb_height));
		const float ortho_projection[4][4] =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};
		glUseProgram(GUI::getGUI().m_shader_handle);
		glUniform1i(GUI::getGUI().m_attrib_location_tex, 0);
		glUniformMatrix4fv(GUI::getGUI().m_attrib_location_proj_mtx, 1, GL_FALSE, &ortho_projection[0][0]);
		glBindVertexArray(GUI::getGUI().m_vao_handle);

		for (int n = 0; n < draw_data->CmdListsCount; ++n)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			glBindBuffer(GL_ARRAY_BUFFER, GUI::getGUI().m_vbo_handle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GUI::getGUI().m_elements_handle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
					glScissor(static_cast<int>(pcmd->ClipRect.x), static_cast<int>(fb_height - pcmd->ClipRect.w), static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x), static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y));
					glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
		}

		//Restore modified GL state
		glUseProgram(last_program);
		glActiveTexture(last_active_texture);
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindVertexArray(last_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFunc(last_blend_src, last_blend_dst);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
	}

	const char* getClipboardText(void* user_data)
	{
		return glfwGetClipboardString(static_cast<GLFWwindow*>(user_data));
	}

	void setClipboardText(void* user_data, const char* text)
	{
		glfwSetClipboardString(static_cast<GLFWwindow*>(user_data), text);
	}

	void mouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/)
	{
		if (action == GLFW_PRESS && button >= 0 && button < 3)
		{
			GUI::getGUI().m_mouse_pressed[button] = true;
		}
	}

	void scrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset)
	{
		//Use fractional mouse wheel, 1.0 unit 5 lines.
		GUI::getGUI().m_mouse_wheel += static_cast<float>(yoffset);
	}

	void keyCallback(GLFWwindow*, int key, int, int action, int mods)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (action == GLFW_PRESS)
		{
			io.KeysDown[key] = true;
		}
		if (action == GLFW_RELEASE)
		{
			io.KeysDown[key] = false;
		}

		(void)mods; //Modifiers are not reliable across systems.
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	}

	void charCallback(GLFWwindow*, unsigned int c)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
		{
			io.AddInputCharacter(static_cast<unsigned short>(c));
		}
	}

	bool GUI::createFontsTexture()
	{
		//Build texture atlas.
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;

		//Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible
		//with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling
		//GetTexDataAsAlpha8() instead to save on GPU memory.
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		//Upload texture to graphics system.
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &m_font_texture);
		glBindTexture(GL_TEXTURE_2D, m_font_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		//Store our identifier.
		io.Fonts->TexID = (void *)(intptr_t)m_font_texture;

		//Restore state.
		glBindTexture(GL_TEXTURE_2D, last_texture);

		return true;
	}

	bool GUI::createDeviceObjects()
	{
		//Backup GL state.
		GLint last_texture, last_array_buffer, last_vertex_array;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		//GLSLProgram class is not used since this wrapper class is wanted to be self-contained.
		const GLchar *vertex_shader =
			"#version 330\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"#version 330\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		m_shader_handle = glCreateProgram();
		m_vert_handle = glCreateShader(GL_VERTEX_SHADER);
		m_frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_vert_handle, 1, &vertex_shader, 0);
		glShaderSource(m_frag_handle, 1, &fragment_shader, 0);
		glCompileShader(m_vert_handle);
		glCompileShader(m_frag_handle);
		glAttachShader(m_shader_handle, m_vert_handle);
		glAttachShader(m_shader_handle, m_frag_handle);
		glLinkProgram(m_shader_handle);

		m_attrib_location_tex = glGetUniformLocation(m_shader_handle, "Texture");
		m_attrib_location_proj_mtx = glGetUniformLocation(m_shader_handle, "ProjMtx");
		m_attrib_location_position = glGetAttribLocation(m_shader_handle, "Position");
		m_attrib_location_uv = glGetAttribLocation(m_shader_handle, "UV");
		m_attrib_location_color = glGetAttribLocation(m_shader_handle, "Color");

		glGenBuffers(1, &m_vbo_handle);
		glGenBuffers(1, &m_elements_handle);

		glGenVertexArrays(1, &m_vao_handle);
		glBindVertexArray(m_vao_handle);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_handle);
		glEnableVertexAttribArray(m_attrib_location_position);
		glEnableVertexAttribArray(m_attrib_location_uv);
		glEnableVertexAttribArray(m_attrib_location_color);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		glVertexAttribPointer(m_attrib_location_position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(m_attrib_location_uv, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(m_attrib_location_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

		createFontsTexture();

		//Restore modified GL state.
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

		return true;
	}

	void GUI::destroyDeviceObjects()
	{
		if (m_vao_handle)
		{
			glDeleteVertexArrays(1, &m_vao_handle);
			m_vao_handle = 0;
		}
		if (m_vbo_handle)
		{
			glDeleteBuffers(1, &m_vbo_handle);
			m_vbo_handle = 0;
		}
		if (m_elements_handle)
		{
			glDeleteBuffers(1, &m_elements_handle);
			m_elements_handle = 0;
		}
		if (m_shader_handle && m_vert_handle)
		{
			glDetachShader(m_shader_handle, m_vert_handle);
		}
		if (m_vert_handle)
		{
			glDeleteShader(m_vert_handle);
			m_vert_handle = 0;
		}
		if (m_shader_handle && m_frag_handle)
		{
			glDetachShader(m_shader_handle, m_frag_handle);
		}
		if (m_frag_handle)
		{
			glDeleteShader(m_frag_handle);
			m_frag_handle = 0;
		}
		if (m_shader_handle)
		{
			glDeleteProgram(m_shader_handle);
			m_shader_handle = 0;
		}
		if (m_font_texture)
		{
			glDeleteTextures(1, &m_font_texture);
			ImGui::GetIO().Fonts->TexID = 0;
			m_font_texture = 0;
		}
	}
}