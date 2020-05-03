#pragma once

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <gl/glew.h>

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath);
	~ShaderProgram();

	GLuint getID();

	void draw();

	void bind();
	void cleanUp();

private:

	unsigned int VAO;
	unsigned int VBO;

	GLuint ID;
	
	GLuint createShader(const char* filePath, GLenum type);

};

