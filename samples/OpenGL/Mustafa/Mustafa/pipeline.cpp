//FRAMEWORK
#include "pipeline.h"

//CPP
#include <iostream>
#include <fstream>
#include <sstream>

Pipeline::~Pipeline()
{
	glDeleteProgram(m_pipeline);
}

void Pipeline::attachShader(ShaderType type, const std::string& shader_path)
{
	std::ifstream shader_file;
	std::string shader_code;

	//Ensure that files can throw exceptions.
	shader_file.exceptions(std::ifstream::badbit);
	try
	{
		std::stringstream vs_stream;
		shader_file.open(shader_path);

		//Read the whole text from the file.
		vs_stream << shader_file.rdbuf();

		shader_file.close();

		//Load vertex and fragment shaders code from the streams.
		shader_code = vs_stream.str();
	}

	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::pipeline.cpp::attachShader::SHADER_FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	GLuint shader = 0;
	switch (type)
	{
	case VERTEX_SHADER:
		shader = glCreateShader(GL_VERTEX_SHADER);
		break;

	case FRAGMENT_SHADER:
		shader = glCreateShader(GL_FRAGMENT_SHADER);
		break;

	default:
		std::cout << "WARNING::pipeline.cpp::attachShader::UNDEFINED_SHADER_TYPE" << std::endl;
		break;
	}

	const GLchar* sc_ptr = shader_code.c_str();
	glShaderSource(shader, 1, &sc_ptr, nullptr);
	glCompileShader(shader);
	m_shaders.push_back(shader);

	//Check errors, if any.
	GLint success;
	GLchar info_log[1024];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 1024, nullptr, info_log);
		std::cout << "ERROR::pipeline.cpp::attachShader::COMPILATION_FAILED\n" << info_log << std::endl;
	}
}

void Pipeline::create()
{
	m_pipeline = glCreateProgram();

	for (auto shader : m_shaders)
	{
		glAttachShader(m_pipeline, shader);
	}

	glLinkProgram(m_pipeline);

	for (auto shader : m_shaders)
	{
		//Delete the shader as it is no longer necessary.
		glDeleteShader(shader);
	}

	//Check errors, if any.
	GLint success;
	GLchar info_log[1024];
	glGetProgramiv(m_pipeline, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_pipeline, 1024, nullptr, info_log);
		std::cout << "ERROR::pipeline.cpp::create::LINKING_FAILED\n" << info_log << std::endl;
	}
}

void Pipeline::use() const
{
	glUseProgram(m_pipeline);
}