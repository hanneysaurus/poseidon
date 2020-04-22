#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include "gl/glew.h"

class Shader
{

public:

	Shader();
	Shader(std::string relativeFilePath, GLenum type);
	~Shader();

	GLuint getID();

private:

	GLuint shaderID;

	GLuint createShader(std::string relativeFilePath, GLenum type);

};

