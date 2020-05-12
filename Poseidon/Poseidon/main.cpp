#define WIN32_LEAN_AND_MEAN
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include "windows.h"

#include "ShaderProgram.h"
#include "Texture.h"

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

void setUpLibraries(void);
void setCallbackFunctions(void);
void setUpShaders(void);

void render(void);
void renderSquare(void);

void cleanUp(void);

// --------------------------------------------------------
// GLOBAL VARIABLES
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Poseidon";
const int REFRESH_RATE = 60;
GLFWwindow* WINDOW;

static float _zoom = 60.0f;

ShaderProgram shaderProgram;
Texture testTex;

int main(int argc, char* argv[])
{

	setUpLibraries();

	setCallbackFunctions();

	setUpShaders();

	// Main loop
	while (!glfwWindowShouldClose(WINDOW))
	{
		glfwPollEvents();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
			
		render();

		// Update Screen
		glfwSwapBuffers(WINDOW);

	}

	cleanUp();

	// Terminate GLFW
	glfwTerminate();

	// Exit program
	exit(EXIT_SUCCESS);
}

void setUpLibraries(void) {
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

}
void setUpShaders(void) {

	shaderProgram = ShaderProgram("../shaders/vertexShader.vert", "../shaders/fragmentShader.frag");
	shaderProgram.addGemoetryShader("../shaders/computeShader.geom");
	shaderProgram.bind();

	testTex = Texture("../textures/container.jpg");
	
}

void setCallbackFunctions(void) {
	glfwSetKeyCallback(WINDOW, key_callback);
	glfwSetScrollCallback(WINDOW, scroll_callback);
}

void render(void) {
	renderSquare();
}

void renderSquare() {

	testTex.use();

	glUseProgram(shaderProgram.getID());

	//set Uniforms

	shaderProgram.draw();
}

void cleanUp() {

	shaderProgram.cleanUp();
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