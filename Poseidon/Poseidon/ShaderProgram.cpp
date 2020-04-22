#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
	this->shaderProgramID = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{

}

void ShaderProgram::linkShaderToProgram(Shader shader)
{
	glAttachShader(this->shaderProgramID, shader.getID());
	glLinkProgram(this->shaderProgramID);
	shader.~Shader();
}
