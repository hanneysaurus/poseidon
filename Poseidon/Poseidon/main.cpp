#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include "windows.h"

#include "ShaderProgram.h"
#include "Shader.h"

#include "GLFW/glfw3.h"
#include "gl/glew.h"
#include <gl/GL.h>		//standard OpenGL include
#include <gl/GLU.h>		//OpenGL utilities
#include "gl/glut.h"
#include "glm/glm.hpp"

// --------------------------------------------------------
// FUNCTION DECLARATIONS
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// --------------------------------------------------------
// GLOBAL VARIABLES
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Poseidon";
const int REFRESH_RATE = 60;
GLFWwindow* WINDOW;

static float _zoom = 60.0f;

Shader vertShader;
Shader fragShader;

int main(int argc, char* argv[])
{

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	WINDOW = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (!WINDOW)
	{
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(WINDOW);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
	}

	// set callbacks
	glfwSetKeyCallback(WINDOW, key_callback);
	glfwSetScrollCallback(WINDOW, scroll_callback);

	//vertShader = Shader("../shaders/vertexShader.vert", GL_VERTEX_SHADER);
	//fragShader = Shader("../shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
	
	const char* vertexShaderSource = "#version 430 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos, 1.0);\n"
		"}\0";

	const char* fragmentShaderSource = "#version 430 core\n"
		"out vec4 FragColor;\n"
		"uniform vec4 ourColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = ourColor;\n"
		"}\n\0";
	
	int success;
	char infoLog[512];

	int vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
		std::cout << "ERROR: Vertex Shader Compilation failed\n" << infoLog << std::endl;
	}

	int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		std::cout << "ERROR: Fragment Shader Compilation failed\n" << infoLog << std::endl;
	}
	
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{

		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR: Shader Linking failed \n" << infoLog << std::endl;
	}
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	float vertices[] =
	{
		0.5f, -0.5f, 0.0,
		-0.5, -0.5, 0.0,
		0.0, 0.5, 0.0
	};
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO);

	// Main loop
	while (!glfwWindowShouldClose(WINDOW))
	{

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);

		float timeValue = glfwGetTime();
		float greenValue = glm::sin(timeValue) / 2.0f + 0.5f;
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
		
		glDrawArrays(GL_TRIANGLES, 0, 3);


		// Update Screen
		glfwSwapBuffers(WINDOW);
		glfwPollEvents();


	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	// Terminate GLFW
	glfwTerminate();

	// Exit program
	exit(EXIT_SUCCESS);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action != GLFW_RELEASE) return;

	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	_zoom -= float(yoffset) * 2.0f;
}