#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::ShaderProgram(const char* computeFilePath)
{
	this->program_id = glCreateProgram();

	GLuint computeShader = createShader(computeFilePath, ShaderType::COMPUTE);

	glAttachShader(program_id, computeShader);
	glLinkProgram(program_id);

	int success;
	char infoLog[512];
	glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(computeShader);
}

ShaderProgram::ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath)
{
	this->program_id = glCreateProgram();

	GLuint vertexShader = createShader(vertexFilePath, ShaderType::VERTEX);
	GLuint fragmentShader = createShader(fragmentFilePath, ShaderType::FRAGMENT);

	glAttachShader(program_id, vertexShader);
	glAttachShader(program_id, fragmentShader);
	glLinkProgram(program_id);

	int success;
	char infoLog[512];
	glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram()
{
	
}

unsigned int ShaderProgram::getID()
{
	return this->program_id;
}


void ShaderProgram::dispatchCompute(int width, int height, int depth)
{
	glDispatchCompute(width, height, depth);
}

void ShaderProgram::use()
{
	glUseProgram(this->program_id);
}

void ShaderProgram::unuse()
{
	glUseProgram(0);
}

GLuint ShaderProgram::createShader(const char* filePath, ShaderType type) {

	GLenum shaderType;
	switch (type)
	{
	case ShaderType::VERTEX:
		shaderType = GL_VERTEX_SHADER;
		break;
	case ShaderType::COMPUTE:
		shaderType = GL_COMPUTE_SHADER;
		break;
	case ShaderType::FRAGMENT:
		shaderType = GL_FRAGMENT_SHADER;
		break;
	default:
		break;
	}

	std::string absoluteFilePath = std::filesystem::absolute(filePath).u8string();
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

	GLuint shader = glCreateShader(shaderType);
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
