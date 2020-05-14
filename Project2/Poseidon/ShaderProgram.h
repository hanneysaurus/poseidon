#pragma once

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <gl/glew.h>
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>


// error logging
//------------------------------------------------------------------------------------------------------------------------


// error logging
//------------------------------------------------------------------------------------------------------------------------
enum class Shader_type {
		VERTEX = 0, COMPUTE = 1, FRAGMENT = 2
	};

class ShaderProgram
{
	
public:
	ShaderProgram();
	ShaderProgram(std::string computeFilePath);
	ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath);
	~ShaderProgram();

	unsigned int getID();

	void bind();
	void unbind();

	int GetUniformLocation(const std::string& name);
	void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3);

	void dispatchCompute(const int width,const int height,const int depth);

private:

	unsigned int program_id;
	std::unordered_map<std::string, int> m_UniformLocationCache;
	
	unsigned int createShader(std::string filePath, Shader_type shader_type);


};
