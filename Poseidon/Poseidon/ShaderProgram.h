#pragma once

#include <gl/glew.h>
#include "Shader.h"

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	void linkShaderToProgram(Shader shader);

private:

	GLuint shaderProgramID;
};

