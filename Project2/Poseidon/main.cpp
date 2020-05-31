#define WIN32_LEAN_AND_MEAN
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include "windows.h"

#include "ShaderProgram.h"
#include "Texture.h"

#include "GLFW/glfw3.h"
#include "gl/glew.h"
#include <gl/GL.h>		// standard OpenGL include
#include <gl/GLU.h>		// OpenGL utilities
#include "gl/glut.h"
#include "glm/glm.hpp"
#include "glm/gtc/noise.hpp"

#include "Debug.h"
#include "UtilityFunctions.h"

// --------------------------------------------------------
// FUNCTION DECLARATIONS

// callback functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// setup functions
void setUpLibraries();
void setCallbackFunctions();
void initialize();

// loop functions
void render();

// cleanup functions
void cleanUp();

// --------------------------------------------------------
// GLOBAL VARIABLES

// window settings
GLFWwindow* window = nullptr;
const int window_width = 800;
const int window_height = 600;
const char* window_title = "Poseidon";

// shaders
ShaderProgram programCompute;
ShaderProgram programRender;
unsigned int VAO, VBO;

// textures
unsigned int texture_read = 0;
unsigned int texture_tilde_h0k = 0;
unsigned int texture_tilde_h0minusk = 0;

unsigned int texture_random_noise_1 = 0;
unsigned int texture_random_noise_2 = 0;
unsigned int texture_random_noise_3 = 0;
unsigned int texture_random_noise_4 = 0;

const unsigned int texture_width = 10;
const unsigned int texture_height = 10;


// --------------------------------------------------------
// STRUCTS
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoord;
};

// --------------------------------------------------------
// MAIN
int main(void)
{
    setUpLibraries();
    setCallbackFunctions();

    initialize();

    //render loop
    while (!glfwWindowShouldClose(window))
    {        
        glEnable(GL_BLEND);
        //glBlendFunc(GL_RGB, GL_SRC_ALPHA);

        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUp();

    glfwTerminate();
    return 0;
}


void initialize()
{
    // create shaderPrograms
    programRender = ShaderProgram("VertexShader.shader", "FragmentShader.shader");
    programCompute = ShaderProgram("ComputeShader.shader");

    //create vertex objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenTextures(1, &texture_read);
    glGenTextures(1, &texture_tilde_h0k);
    glGenTextures(1, &texture_tilde_h0minusk);
    glGenTextures(1, &texture_random_noise_1);
    glGenTextures(1, &texture_random_noise_2);
    glGenTextures(1, &texture_random_noise_3);
    glGenTextures(1, &texture_random_noise_4);

    // setup vertex array
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    // setup vertex buffer
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


    // setup texture that is written to in compute shader and then read from in fragment shader
    glBindTexture(GL_TEXTURE_2D, texture_tilde_h0k);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, texture_width, texture_height);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, texture_tilde_h0minusk);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, texture_width, texture_height);
    glBindTexture(GL_TEXTURE_2D, 0);


    // create noise data:
    GLubyte random_noise_1_data[texture_width][texture_width][4];
    for (int i = 0; i < texture_width; i++) {
        for (int j = 0; j < texture_height; j++) {

            int rand_value = rand() % 255;

            random_noise_1_data[i][j][0] =  rand_value; // RED
            random_noise_1_data[i][j][1] =  rand_value; // GREEN 
            random_noise_1_data[i][j][2] =  rand_value; // BLUE
            random_noise_1_data[i][j][3] = 255;         //ALPHA
        }
    }

    GLubyte random_noise_2_data[texture_width][texture_width][4];
    for (int i = 0; i < texture_width; i++) {
        for (int j = 0; j < texture_height; j++) {

            int rand_value = rand() % 255;

            random_noise_2_data[i][j][0] = rand_value;
            random_noise_2_data[i][j][1] = rand_value;
            random_noise_2_data[i][j][2] = rand_value;
            random_noise_2_data[i][j][3] = 255;
        }
    }

    GLubyte random_noise_3_data[texture_width][texture_width][4];
    for (int i = 0; i < texture_width; i++) {
        for (int j = 0; j < texture_height; j++) {

            int rand_value = rand() % 255;

            random_noise_3_data[i][j][0] = rand_value;
            random_noise_3_data[i][j][1] = rand_value;
            random_noise_3_data[i][j][2] = rand_value;
            random_noise_3_data[i][j][3] = 255;
        }
    }

    GLubyte random_noise_4_data[texture_width][texture_width][4];
    for (int i = 0; i < texture_width; i++) {
        for (int j = 0; j < texture_height; j++) {

            int rand_value = rand() % 255;

            random_noise_4_data[i][j][0] = rand_value;
            random_noise_4_data[i][j][1] = rand_value;
            random_noise_4_data[i][j][2] = rand_value;
            random_noise_4_data[i][j][3] = 255;
        }
    }

    // create and bind textues to store noise data
    glBindTexture(GL_TEXTURE_2D, texture_random_noise_1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_width, texture_height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, random_noise_1_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, texture_random_noise_2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_width, texture_height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, random_noise_2_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, texture_random_noise_3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_width, texture_height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, random_noise_3_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, texture_random_noise_4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_width, texture_height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, random_noise_4_data);
    glBindTexture(GL_TEXTURE_2D, 0);


    // connect texture to write to as image variable in compute shader
    glBindImageTexture(0, texture_tilde_h0k, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_tilde_h0minusk, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // connect texture to read from as sampler in fragment shader
    unsigned int texture_unit = 1;
    glBindTextureUnit(texture_unit, texture_tilde_h0k);
    int location = glGetUniformLocation(programRender.getID(), "tex1");
    glProgramUniform1i(programRender.getID(), location, texture_unit);

    texture_unit = 2;
    glBindTextureUnit(texture_unit, texture_tilde_h0minusk);
    location = glGetUniformLocation(programRender.getID(), "tex2");
    glProgramUniform1i(programRender.getID(), location, texture_unit);

    glBindImageTexture(3, texture_random_noise_1, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programCompute.getID(), "randtex1");
    glProgramUniform1i(programCompute.getID(), location, 3);

    glBindImageTexture(4, texture_random_noise_2, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programCompute.getID(), "randtex2");
    glProgramUniform1i(programCompute.getID(), location, 4);

    glBindImageTexture(5, texture_random_noise_3, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programCompute.getID(), "randtex3");
    glProgramUniform1i(programCompute.getID(), location, 5);

    glBindImageTexture(6, texture_random_noise_4, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programCompute.getID(), "randtex4");
    glProgramUniform1i(programCompute.getID(), location, 6);


    //invoke compute shader
    programCompute.bind();
    programCompute.dispatchCompute(texture_width, texture_height, 1);
    programCompute.unbind();
}

void setUpLibraries()
{
    //initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
    }

    //create window
    window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
    }

    //set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwMakeContextCurrent(window);

    //initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
    }
}

void setCallbackFunctions(void) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
}


void render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0);    // background = gray
    glClear(GL_COLOR_BUFFER_BIT);

    // render quad
    programRender.bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
    programRender.unbind();
}


void cleanUp()
{
    // delete all objects
    glDeleteProgram(programCompute.getID());
    glDeleteProgram(programRender.getID());
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture_read);
    glDeleteTextures(1, &texture_tilde_h0k);
    glDeleteTextures(1, &texture_tilde_h0minusk);
    glDeleteTextures(1, &texture_random_noise_1);
    glDeleteTextures(1, &texture_random_noise_2);
    glDeleteTextures(1, &texture_random_noise_3);
    glDeleteTextures(1, &texture_random_noise_4);
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
  
}



// STORAGE AREA FOR OLD CODE: 

// noise texture generation -> replaced with cooler syntax! /alvin
 ////random noise textures
    //unsigned char random_noise_1_data[10 * 10 * 4];
    //for (int row = 0; row < texture_height; row++)    
    //{
    //    for (int col = 0; col < texture_width; col++)
    //    {
    //        int rand_value = rand() % 255;
    //       
    //        random_noise_1_data[((row * texture_width + col) * 4) + 0] = rand_value;
    //        random_noise_1_data[((row * texture_width + col) * 4) + 1] = rand_value;
    //        random_noise_1_data[((row * texture_width + col) * 4) + 2] = rand_value;
    //        random_noise_1_data[((row * texture_width + col) * 4) + 3] = 255; 
    //    }
    //}

    //unsigned char random_noise_2_data[10 * 10 * 4];
    //for (int row = 0; row < texture_height; row++)
    //{
    //    for (int col = 0; col < texture_width; col++)
    //    {
    //        int rand_value = rand() % 255;

    //        random_noise_2_data[((row * texture_width + col) * 4) + 0] = rand_value;
    //        random_noise_2_data[((row * texture_width + col) * 4) + 1] = rand_value;
    //        random_noise_2_data[((row * texture_width + col) * 4) + 2] = rand_value;
    //        random_noise_2_data[((row * texture_width + col) * 4) + 3] = 255;
    //    }
    //}

    //unsigned char random_noise_3_data[10 * 10 * 4];
    //for (int row = 0; row < texture_height; row++)
    //{
    //    for (int col = 0; col < texture_width; col++)
    //    {
    //        int rand_value = rand() % 255;

    //        random_noise_3_data[((row * texture_width + col) * 4) + 0] = rand_value;
    //        random_noise_3_data[((row * texture_width + col) * 4) + 1] = rand_value;
    //        random_noise_3_data[((row * texture_width + col) * 4) + 2] = rand_value;
    //        random_noise_3_data[((row * texture_width + col) * 4) + 3] = 255;
    //    }
    //}

    //unsigned char random_noise_4_data[10 * 10 * 4];
    //for (int row = 0; row < texture_height; row++)
    //{
    //    for (int col = 0; col < texture_width; col++)
    //    {
    //        int rand_value = rand() % 255;

    //        random_noise_4_data[((row * texture_width + col) * 4) + 0] = rand_value;
    //        random_noise_4_data[((row * texture_width + col) * 4) + 1] = rand_value;
    //        random_noise_4_data[((row * texture_width + col) * 4) + 2] = rand_value;
    //        random_noise_4_data[((row * texture_width + col) * 4) + 3] = 255;
    //    }
    //}