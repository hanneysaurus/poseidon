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

#include <bitset> 


#define INT_SIZE 32


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

unsigned int reverseBits(unsigned int n);

// cleanup functions
void cleanUp();

// --------------------------------------------------------
// GLOBAL VARIABLES

// window settings
GLFWwindow* window = nullptr;
const int window_width = 800;
const int window_height = 800;
const char* window_title = "Poseidon";

// shaders
ShaderProgram programTildeHCompute;
ShaderProgram programRender;
ShaderProgram programButterflyTextureCompute;
ShaderProgram programFourierComponentCompute;
unsigned int VAO, VBO;

// textures
const unsigned int texture_width = 256;
const unsigned int texture_height = 256;

Texture texture_random_noise_1;
Texture texture_random_noise_2;
Texture texture_random_noise_3;
Texture texture_random_noise_4;

Texture texture_tilde_h0k;
Texture texture_tilde_h0minusk;
Texture texture_butterfly;
Texture texture_fourier_component_dx;
Texture texture_fourier_component_dy;
Texture texture_fourier_component_dz;

int* bitReversedIndices;

//uniform variables
//float time = 0.0f;

int N = 256;
int L = 2048;
float A = 4;
glm::vec2 windDirection = glm::vec2(1.0f, 1.0f);
float windSpeed = 40;

template <typename T>
T rol_(T value, int count) {
    return (value << count) | (value >> (sizeof(T) * CHAR_BIT - count));
}

// --------------------------------------------------------
// STRUCTS
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoord;
};


template <std::size_t N>
inline int rotate(std::bitset<N>& b, unsigned m)
{
    b = b << m | b >> (N - m);
    return (int)(b.to_ulong());
}

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
    programTildeHCompute = ShaderProgram("tildehcompute.shader");
    programButterflyTextureCompute = ShaderProgram("butterflyTextureCompute.shader");
    programFourierComponentCompute = ShaderProgram("fourierComponentCompute.shader");

    int location;


    //TILDEHCOMPUTE
    texture_tilde_h0k = Texture(false, texture_width, texture_height);
    glBindImageTexture(0, texture_tilde_h0k.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(0, texture_tilde_h0k.getID());
    location = glGetUniformLocation(programRender.getID(), "tex1");
    glProgramUniform1i(programRender.getID(), location, 0);

    texture_tilde_h0minusk = Texture(false, texture_width, texture_height);
    glBindImageTexture(1, texture_tilde_h0minusk.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(1, texture_tilde_h0minusk.getID());
    location = glGetUniformLocation(programRender.getID(), "tex2");
    glProgramUniform1i(programRender.getID(), location, 1);

    texture_random_noise_1 = Texture(true, texture_width, texture_height);
    glBindImageTexture(3, texture_random_noise_1.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex1");
    glProgramUniform1i(programTildeHCompute.getID(), location, 3);

    texture_random_noise_2 = Texture(true, texture_width, texture_height);
    glBindImageTexture(4, texture_random_noise_2.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex2");
    glProgramUniform1i(programTildeHCompute.getID(), location, 4);

    texture_random_noise_3 = Texture(true, texture_width, texture_height);
    glBindImageTexture(5, texture_random_noise_3.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex3");
    glProgramUniform1i(programTildeHCompute.getID(), location, 5);

    texture_random_noise_4 = Texture(true, texture_width, texture_height);
    glBindImageTexture(6, texture_random_noise_4.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex4");
    glProgramUniform1i(programTildeHCompute.getID(), location, 6);

    programTildeHCompute.bind();
    programTildeHCompute.dispatchCompute(texture_width, texture_height, 1);
    programTildeHCompute.unbind();




    // BUTTERFLY
    bitReversedIndices = new int[texture_height];
    int bits = (log(texture_height) / log(2));
    for (int i = 0; i < texture_height; i++)
    {
        unsigned int x = reverseBits(i);
        x = (x << bits) | (x >> (INT_SIZE - bits));
        bitReversedIndices[i] = x;
    }

    // Create the buffer that passes bitReversedIndices to butterfly compute shader
    unsigned int reverseIndicesSSBO;
    glGenBuffers(1, &reverseIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reverseIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * texture_height, bitReversedIndices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, reverseIndicesSSBO); // buffer assigned to binding index 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind the buffer

    texture_butterfly = Texture(false, log(texture_width) / log(2), texture_height);
    glBindImageTexture(2, texture_butterfly.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(2, texture_butterfly.getID());
    location = glGetUniformLocation(programRender.getID(), "butterfly_texture");
    glProgramUniform1i(programRender.getID(), location, 2);

    programButterflyTextureCompute.bind();
    programButterflyTextureCompute.dispatchCompute(texture_width, texture_height, 1);
    programButterflyTextureCompute.unbind();




    //FOURIER
    location = glGetUniformLocation(programFourierComponentCompute.getID(), "time");
    glUniform1f(location, 0.0f);

    location = glGetUniformLocation(programFourierComponentCompute.getID(), "N");
    glUniform1i(location, 1);

    location = glGetUniformLocation(programFourierComponentCompute.getID(), "L");
    glUniform1i(location, 1);

    texture_fourier_component_dx = Texture(false, texture_width, texture_height);
    glBindImageTexture(3, texture_fourier_component_dx.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(7, texture_fourier_component_dx.getID());
    location = glGetUniformLocation(programRender.getID(), "fourier_component_dx");
    glProgramUniform1i(programRender.getID(), location, 7);

    texture_fourier_component_dy = Texture(false, texture_width, texture_height);
    glBindImageTexture(4, texture_fourier_component_dy.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(8, texture_fourier_component_dy.getID());
    location = glGetUniformLocation(programRender.getID(), "fourier_component_dy");
    glProgramUniform1i(programRender.getID(), location, 8);

    texture_fourier_component_dz = Texture(false, texture_width, texture_height);
    glBindImageTexture(5, texture_fourier_component_dz.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(9, texture_fourier_component_dz.getID());
    location = glGetUniformLocation(programRender.getID(), "fourier_component_dz");
    glProgramUniform1i(programRender.getID(), location, 9);

    programFourierComponentCompute.bind();
    programFourierComponentCompute.dispatchCompute(texture_width, texture_height, 1);
    programFourierComponentCompute.unbind();





    //create vertex objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

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
    glDeleteProgram(programTildeHCompute.getID());
    glDeleteProgram(programRender.getID());
    glDeleteProgram(programButterflyTextureCompute.getID());

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    texture_random_noise_1.deleteTexture();
    texture_random_noise_2.deleteTexture();
    texture_random_noise_3.deleteTexture();
    texture_random_noise_4.deleteTexture();

    texture_tilde_h0k.deleteTexture();
    texture_tilde_h0minusk.deleteTexture();
    texture_butterfly.deleteTexture();

    // delete array on heap 
    delete bitReversedIndices;
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

unsigned int reverseBits(unsigned int num)
{
    unsigned int  NO_OF_BITS = sizeof(num) * 8;
    unsigned int reverse_num = 0;
    int i;
    for (i = 0; i < NO_OF_BITS; i++)
    {
        if ((num & (1 << i)))
            reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}