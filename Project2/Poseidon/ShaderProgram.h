#pragma once

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <gl/glew.h>

enum class ShaderType {
	VERTEX = 0, COMPUTE = 1, FRAGMENT = 2
};

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(const char* computeFilePath);
	ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath);
	~ShaderProgram();

	unsigned int getID();

	void dispatchCompute(int width, int height, int depth);

	void bind();
	void unbind();

private:

	unsigned int program_id;
	GLuint createShader(const char* filePath, ShaderType type);

};