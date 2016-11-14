#pragma once

//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include <GL/glew.h>

//CPP
#include <string>
#include <vector>

class Material
{
public:
	//To be updated if another shader is needed.
	enum ShaderType
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER
	};

public:
	Material() = default;
	~Material();

	void attachShader(ShaderType type, const std::string& shader_path);
	void link();
	void use() const;

	//To be removed if addUniform() kinda functions are added.
	inline GLuint get_pipeline() const { return m_pipeline; }

private:
	GLuint m_pipeline;
	std::vector<GLuint> m_shaders;
};