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


#include "Shader.h"

// main functions
void Initialize();
void Render();
void CleanUp();

GLFWwindow* window = nullptr;

//unsigned int program_compute = 0;
//unsigned int computeshader = 0;

unsigned int program_render = 0;
unsigned int vertexshader = 0;
unsigned int fragmentshader = 0;
unsigned int vertexarray = 0;
unsigned int vertexbuffer = 0;
unsigned int VAO, VBO;

unsigned int texture_read = 0;
unsigned int texture_draw = 0;
unsigned int texturewidth = 10;
unsigned int textureheight = 10;

ShaderProgram computeProgram;

//------------------------------------------------------------------------------------------------------------------------



std::string computeshader_source = {
    "#version 450 core\n"
    "layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;"
    "layout ( binding = 3, rgba32f ) uniform writeonly image2D img2;"
    "void main() {"
    "ivec2 texel = ivec2(gl_GlobalInvocationID.xy);"
    "vec4 color2 = vec4((texel.x+1.0)/10.0,(texel.y+1.0)/10.0,(texel.x+1.0)/10.0,1.0);"
    "imageStore(img2, texel, color2);"
    "}"
};

std::string vertexshader_source = {
    "#version 450 core\n"
    "in layout (location = 0) vec3 in_position;"
    "in layout (location = 1) vec2 in_texcoord;"
    "out vec2 texcoord;"
    "void main() {"
    "gl_Position = vec4(in_position, 1);"
    "texcoord = in_texcoord;"
    "}"
};

std::string fragmentshader_source = {
    "#version 450 core\n"
    "in vec2 texcoord;"
    "uniform sampler2D tex1;"
    "out layout (location = 0) vec4 out_color;"
    "void main() {"
    "out_color = texture(tex1, texcoord);"
    "}"
};


struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoord;
};


int main(void)
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Example code", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Initialize GLEW */
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        return -1;
    }

    computeProgram = ShaderProgram("../shaders/computeShader.comp");

    Initialize();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        Render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    CleanUp();

    glfwTerminate();
    return 0;
}


void Initialize()
{
    // create all objects
    //------------------------------------------------------------------------------------------------------------------------
    //program_compute = glCreateProgram();
    program_render = glCreateProgram();
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    //computeshader = glCreateShader(GL_COMPUTE_SHADER);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenTextures(1, &texture_read);
    glGenTextures(1, &texture_draw);
    //------------------------------------------------------------------------------------------------------------------------

    // setup programs
    //------------------------------------------------------------------------------------------------------------------------
    // compute
    //CompileShader(computeshader, computeshader_source);
    //LinkProgram(program_compute, { computeshader });

    // render
    CompileShader(vertexshader, vertexshader_source);
    CompileShader(fragmentshader, fragmentshader_source);
    LinkProgram(program_render, { vertexshader, fragmentshader });
    //------------------------------------------------------------------------------------------------------------------------


    // setup vertex array
    //------------------------------------------------------------------------------------------------------------------------
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------------------------------------------


    // setup vertex buffer
    //------------------------------------------------------------------------------------------------------------------------
    // quad
    Vertex vertices[] = {
        //    x           y       z      u  v
        { { -1.0f,    -1.0f,    0 },{ 0, 0 } }, // bottom-left
        { { +1.0f,    -1.0f,    0 },{ 1, 0 } }, // bottom-right
        { { +1.0f,    +1.0f,    0 },{ 1, 1 } }, // top-right
        { { -1.0f,    +1.0f,    0 },{ 0, 1 } }, // top-left
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //------------------------------------------------------------------------------------------------------------------------

    // setup texture that is written to in compute shader and then read from in fragment shader
    //------------------------------------------------------------------------------------------------------------------------

    glBindTexture(GL_TEXTURE_2D, texture_draw);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, texturewidth, textureheight);
    glBindTexture(GL_TEXTURE_2D, 0);

    // connect texture to write to as image variable in compute shader
    unsigned int image_unit = 3;
    glBindImageTexture(image_unit, texture_draw, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // connect texture to read from as sampler in fragment shader
    unsigned int texture_unit = 1;
    glBindTextureUnit(texture_unit, texture_draw);
    int location = glGetUniformLocation(program_render, "tex1");
    glProgramUniform1i(program_render, location, texture_unit);

    //------------------------------------------------------------------------------------------------------------------------

    // invoke compute shader
    //------------------------------------------------------------------------------------------------------------------------

    
    //glUseProgram(computeProgram.getID());
    computeProgram.bind();
    computeProgram.dispatchCompute(texturewidth, textureheight, 1);
    //glDispatchCompute(texturewidth, textureheight, 1);
    //glUseProgram(0);
    computeProgram.unbind();

    std::cout << "AM I CRAZY? ";
}


void Render()
{
    glClearColor(0.5f, 0.5f, 0.5f, 0);    // background = gray
    glClear(GL_COLOR_BUFFER_BIT);

    // render quad
    glUseProgram(program_render);
    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

}


void CleanUp()
{
    // delete all objects
    //glDeleteProgram(program_compute);
    glDeleteProgram(program_render);
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    //glDeleteShader(computeshader);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture_read);
    glDeleteTextures(1, &texture_draw);
}


















































/*#define WIN32_LEAN_AND_MEAN
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
// ERROR LOGGING
#define ASSERT(x) if ((!x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))


static void GLClearError(){
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()){
        std::cout << "openGL error: (" << error << "): function" << function  << std::endl << " file: " << file <<  std::endl << " line: " << line  << std::endl;
        return false;
    }
    return true;
}


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
ShaderProgram computeProgram;
unsigned int VAO, VBO, EBO;
unsigned int read_write_texture;

float testingWaters[] = {
	//position			//uv coords
    0.5f, 0.5f, 0.0f,	1.0f, 1.0f, // top-right
	0.5f, -0.5f, 0.0f,	1.0f, 0.0f, // bottom-right
   -0.5f, 0.5f, 0.0f,	0.0f, 1.0f, // top-left
   -0.5f, -0.5f, 0.0f,	0.0f, 0.0f, // bottom-left
};

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	WINDOW = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (!WINDOW)
	{
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(WINDOW);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
	}

}
void setUpShaders(void) {
	
	glGenTextures(1, &read_write_texture);
	glBindTexture(GL_TEXTURE_2D, read_write_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned int image_unit = 3;
	//glBindImageTexture(image_unit, read_write_texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8);
	unsigned int location = glGetUniformLocation(computeProgram.getID(), "img1");
	glProgramUniform1i(computeProgram.getID(), location, image_unit);

	computeProgram = ShaderProgram("../shaders/computeShader.comp");
	computeProgram.bind();

	shaderProgram = ShaderProgram("../shaders/vertexShader.vert", "../shaders/fragmentShader.frag");
	shaderProgram.bind();

	//testTex = Texture("../textures/container.jpg");
	
}

void setCallbackFunctions(void) {
	glfwSetKeyCallback(WINDOW, key_callback);
	glfwSetScrollCallback(WINDOW, scroll_callback);
}

void render(void) {
	renderSquare();
}

void renderSquare() {

	glUseProgram(computeProgram.getID());
	glDispatchCompute(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
	glUseProgram(0);

	unsigned int texture_unit = 1;
	glBindTextureUnit(texture_unit, read_write_texture);
	unsigned int location = glGetUniformLocation(shaderProgram.getID(), "tex1");
	glProgramUniform1i(shaderProgram.getID(), location, texture_unit);
	glUseProgram(shaderProgram.getID());
	
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

	glDrawArrays(GL_QUADS, 0, 4);
	glUseProgram(0);

}

void cleanUp() {

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
}*/