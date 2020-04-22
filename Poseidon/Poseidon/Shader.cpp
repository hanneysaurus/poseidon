#include "Shader.h"

Shader::Shader()
{
}

Shader::Shader(std::string relativeFilePath, GLenum type)
{
	this->shaderID = createShader(relativeFilePath, type);	
}

Shader::~Shader()
{
	glDeleteShader(this->shaderID);
}

GLuint Shader::getID()
{
	return this->shaderID;
}

GLuint Shader::createShader(std::string relativeFilePath, GLenum type)
{

	std::string absoluteFilePath = std::filesystem::absolute(relativeFilePath).u8string();
	std::ifstream filestream(absoluteFilePath, std::ios::in);

	std::string content;

	std::string line = "";
	while (!filestream.eof())
	{
		std::getline(filestream, line);
		content.append(line + "\n");
	}
	filestream.close();
	const char* shaderSource = content.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);

	glCompileShader(shader);
	GLint succeded;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &succeded);

	if (succeded == GL_FALSE)
	{
		//logging
		GLint logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		GLchar* message = new char[logSize];
		glGetShaderInfoLog(shader, logSize, NULL, message);

		std::cout << message << std::endl;
		delete[] message;
	}

	return shader;

}