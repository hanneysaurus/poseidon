#include "ShaderProgram.h"

float square_vertices[] =
{
	0.5f, -0.5f, 0.0,
	0.5, 0.5, 0.0,
	-0.5, 0.5, 0.0,

	0.5f, -0.5f, 0.0,
	-0.5, -0.5, 0.0,
	-0.5, 0.5, 0.0
};

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::ShaderProgram(const char* vertexFilePath, const char* fragmentFilePath)
{
	this->ID = glCreateProgram();

	GLuint vertexShader = createShader(vertexFilePath, GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader(fragmentFilePath, GL_FRAGMENT_SHADER);

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	int success;
	char infoLog[512];
	glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->ID, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram()
{

}

GLuint ShaderProgram::getID()
{
	return this->ID;
}

void ShaderProgram::draw() {
	glDrawArrays(GL_TRIANGLES, 0, sizeof(square_vertices));
}

void ShaderProgram::bind()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO);
}

void ShaderProgram::cleanUp() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(this->ID);
}

GLuint ShaderProgram::createShader(const char* filePath, GLenum type) {
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
