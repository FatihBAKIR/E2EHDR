#pragma once

//GL
#include <GL/glew.h>

//CPP
#include <string>
#include <vector>

class Pipeline
{
public:
	//To be updated if another shader is needed.
	enum ShaderType
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER
	};

public:
	Pipeline() = default;
	~Pipeline();

	void attachShader(ShaderType type, const std::string& shader_path);
	void create();
	void use() const;

	//To be removed if addUniform() kinda functions are added.
	inline GLuint get_pipeline() const { return m_pipeline; }

private:
	GLuint m_pipeline;
	std::vector<GLuint> m_shaders;
};