#include "ShaderProgram.h"

float square_vertices[] = {
	// positions          // colors           // texture coords
	 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
};

unsigned int square_indices[] = {
	0,1,3,	//first triangle
	1,2,3	//second triangle
};

float testingWaters[] = {
		  //position			//uv coords
		  0.5f, 0.5f, 0.0f,		1.0f, 1.0f, // top-right
		  0.5f,	-0.5f, 0.0f,	1.0f, 0.0f, // bottom-right
		 -0.5f, 0.5f, 0.0f,		0.0f, 1.0f, // top-left
		 -0.5f, -0.5f, 0.0f,	0.0f, 0.0f, // bottom-left
};

ShaderProgram::ShaderProgram()
{

}

ShaderProgram::ShaderProgram(const char* computeFilePath)
{
	this->ID = glCreateProgram();
	addComputeShader(computeFilePath);
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

void ShaderProgram::addComputeShader(const char* computeFilePath)
{
	GLuint computeShader = createShader(computeFilePath, GL_COMPUTE_SHADER);
	glAttachShader(this->ID, computeShader);
	glLinkProgram(this->ID);

	int success;
	char infoLog[512];
	glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->ID, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(computeShader);	

}

void ShaderProgram::addGeometryShader(const char* geometryFilePath)
{
	GLuint geometryShader = createShader(geometryFilePath, GL_GEOMETRY_SHADER);
	glAttachShader(this->ID, geometryShader);
	glLinkProgram(this->ID);

	int success;
	char infoLog[512];
	glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(this->ID, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}

	glDeleteShader(geometryShader);

}

GLuint ShaderProgram::getID()
{
	return this->ID;
}

unsigned int ShaderProgram::getVAO()
{
	return this->VAO;
}

unsigned int ShaderProgram::getVBO()
{
	return this->VBO;
}

void ShaderProgram::draw() {
	glBindVertexArray(this->VAO);
	glDrawArrays(GL_QUADS, 0, 4);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void ShaderProgram::bind()
{
	/*glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);*/

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(testingWaters), &testingWaters, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
}

void ShaderProgram::cleanUp() {
	glDeleteVertexArrays(1, &this->VAO);
	glDeleteBuffers(1, &this->VBO);
	glDeleteBuffers(1, &this->EBO);
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
