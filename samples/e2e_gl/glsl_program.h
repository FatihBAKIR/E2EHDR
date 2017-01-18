#pragma once

//GL
#include "include\glad\glad.h"

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
		GLSLProgram(GLSLProgram&) = delete;
		GLSLProgram(GLSLProgram&& rhs);
		~GLSLProgram();

		GLSLProgram& operator=(GLSLProgram&) = delete;
		GLSLProgram& operator=(GLSLProgram&&);

		void attachShader(ShaderType type, const std::string& shader_path);
		void link();
		void use() const;

		void setUniformIVar(const std::string& name, std::initializer_list<GLint> values) const;
		void setUniformFVar(const std::string& name, std::initializer_list<GLfloat> values) const;

		void setUniformArray(const std::string& name, const std::vector<float>& arr);

		void clear();

	private:
		GLuint m_program;
		std::vector<GLuint> m_shaders;
	};
}