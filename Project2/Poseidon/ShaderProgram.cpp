#include "ShaderProgram.h"

//float square_vertices[] = {
	// positions          // colors           // texture coords
	 //0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	 //0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	//-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	//-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
//};

//unsigned int square_indices[] = {
	//0,1,3,	//first triangle
	//1,2,3	//second triangle
//};

//float testingwaters[] = {
		  //position			//uv coords
		 //0.5f, 0.5f, 0.0f,		1.0f, 1.0f, // top-right
		  //0.5f,	-0.5f, 0.0f,	1.0f, 0.0f, // bottom-right
		 //-0.5f, 0.5f, 0.0f,		0.0f, 1.0f, // top-left
		 //-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, // bottom-left
//};



ShaderProgram::ShaderProgram()
{

}

ShaderProgram::ShaderProgram(std::string computeFilePath)
{
	this->program_id = glCreateProgram();
	unsigned int computeShader = createShader(computeFilePath, Shader_type::COMPUTE);
	glAttachShader(this->program_id, computeShader);
	glLinkProgram(this->program_id);
	glDetachShader(this->program_id, computeShader);

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

	GLuint vertexShader = createShader(vertexFilePath, Shader_type::VERTEX);
	GLuint fragmentShader = createShader(fragmentFilePath, Shader_type::FRAGMENT);

	glAttachShader(this->program_id, vertexShader);
	glAttachShader(this->program_id, fragmentShader);
	glLinkProgram(this->program_id);

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
	glDeleteProgram(this->program_id);
}

GLuint ShaderProgram::getID()
{
	return this->program_id;
}

void ShaderProgram::bind()
{
	glUseProgram(this->program_id);
}


void ShaderProgram::unbind(){
	glUseProgram(0);
}

unsigned int ShaderProgram::createShader(std::string filePath, Shader_type shader_type) {
	GLenum type;
	switch (shader_type)
	{
	case Shader_type::VERTEX:
		type = GL_VERTEX_SHADER;
		break;
	case Shader_type::COMPUTE:
		type = GL_COMPUTE_SHADER;
		break;
	case Shader_type::FRAGMENT:
		type = GL_FRAGMENT_SHADER;
		break;
	default:
		break;
	};

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

	unsigned int shader = glCreateShader(type);
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


int ShaderProgram::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

	int location = glGetUniformLocation(program_id, name.c_str());
    if (location == -1)
        std::cout << "No active uniform variable with name " << name << " found" << std::endl;

    m_UniformLocationCache[name] = location;

    return location;
}

void ShaderProgram::SetUniform1i(const std::string& name, int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform1f(const std::string& name, float value)
{
	glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3)
{
	glUniform4f(GetUniformLocation(name), f0, f1, f2, f3);
}

void ShaderProgram::dispatchCompute(const int width, const int height, const int depth)
{
	glDispatchCompute(width, height, depth);
}

