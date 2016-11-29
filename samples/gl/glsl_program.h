#pragma once

//GL
#include <GL/glew.h>

//CPP
#include <string>
#include <vector>

namespace e2e
{
	class GLSLProgram
	{
	public:
		//To be updated if another shader is needed.
		enum ShaderType
		{
			VERTEX_SHADER,
			FRAGMENT_SHADER
		};

	public:
		GLSLProgram();
		~GLSLProgram();

		void attachShader(ShaderType type, const std::string& shader_path);
		void link();
		void use() const;

		void setUniformIVar(const std::string& name, std::initializer_list<GLint> values) const;
		void setUniformFVar(const std::string& name, std::initializer_list<GLfloat> values) const;

	private:
		GLuint m_program;
		std::vector<GLuint> m_shaders;
	};
}