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
	ShaderProgram(const char* computeFilePath);
	ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath);
	~ShaderProgram();

	void addComputeShader(const char* computeFilePath);

	void addGeometryShader(const char* geometryFilePath);

	GLuint getID();
	unsigned int getVAO();
	unsigned int getVBO();

	void draw();

	void bind();
	void cleanUp();

private:

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	GLuint ID;
	
	GLuint createShader(const char* filePath, GLenum type);

};

