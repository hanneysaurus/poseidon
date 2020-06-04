#define WIN32_LEAN_AND_MEAN
#define STB_IMAGE_IMPLEMENTATION
#define INT_SIZE 32

#include <iostream>
#include "OpenGLImports.h"

#include "ShaderProgram.h"
#include "Texture.h"

#include "Debug.h"
#include "UtilityFunctions.h"

#include <bitset> 


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

// help functions 
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

// help variables for code readability
unsigned int location;

// uniform variables
float fourier_comp_time=0.0f;
int fourier_comp_N = 256;
int fourier_comp_L = 2048;
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
    // create shaders
    programRender = ShaderProgram("VertexShader.shader", "FragmentShader.shader");
    programTildeHCompute = ShaderProgram("tildehcompute.shader");
    programButterflyTextureCompute = ShaderProgram("butterflyTextureCompute.shader");
    programFourierComponentCompute = ShaderProgram("fourierComponentCompute.shader");

    // create vertex objects
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

    // setup vertex buffer ( mapping from coordinates (x,y) to texture coord (u,v)
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

    // --------------------------------------------------------
    /** CREATING TILDE_H0K & TILDE_H0MINUSK TEXTURES */

    // bind tilde_hok & tilde_hominusk textures to fragment shader */
    texture_tilde_h0k = Texture(false, texture_width, texture_height);
    glBindImageTexture(0, texture_tilde_h0k.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(0, texture_tilde_h0k.getID());
    programRender.SetUniform1i("tilde_h0k",0);

    texture_tilde_h0minusk = Texture(false, texture_width, texture_height);
    glBindImageTexture(1, texture_tilde_h0minusk.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(1, texture_tilde_h0minusk.getID());
    programRender.SetUniform1i("h0minusk", 1);

    // bind noise textures as read textures in TildeH compute shader 
    texture_random_noise_1 = Texture(true, texture_width, texture_height);
    glBindImageTexture(3, texture_random_noise_1.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    programTildeHCompute.SetUniform1i("randtex1", 3);

    texture_random_noise_2 = Texture(true, texture_width, texture_height);
    glBindImageTexture(4, texture_random_noise_2.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    programTildeHCompute.SetUniform1i("randtex2", 4);

    texture_random_noise_3 = Texture(true, texture_width, texture_height);
    glBindImageTexture(5, texture_random_noise_3.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    programTildeHCompute.SetUniform1i("randtex3", 5);

    texture_random_noise_4 = Texture(true, texture_width, texture_height);
    glBindImageTexture(6, texture_random_noise_4.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    programTildeHCompute.SetUniform1i("randtex4", 6);

    programTildeHCompute.bindComputeUnbind(texture_width,texture_height);

    // --------------------------------------------------------
    /** CREATING BUTTERFLY TEXTURE */

    bitReversedIndices = new int[texture_height];
    int bits = (log(texture_height) / log(2));
    for (int i = 0; i < texture_height; i++)
    {
        unsigned int x = reverseBits(i);
        x = (x << bits) | (x >> (INT_SIZE - bits));
        bitReversedIndices[i] = x;
    }

    // create the buffer that passes bitReversedIndices to butterfly compute shader & bind to butterfly texture compute shader
    unsigned int reverseIndicesSSBO;
    glGenBuffers(1, &reverseIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reverseIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * texture_height, bitReversedIndices, GL_STATIC_DRAW);
    delete[] bitReversedIndices;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, reverseIndicesSSBO); // buffer assigned to binding index 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind the buffer after use

    texture_butterfly = Texture(false, log(texture_width) / log(2), texture_height);
    glBindImageTexture(2, texture_butterfly.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(2, texture_butterfly.getID());
    programRender.SetUniform1i("butterfly_texture",2);

    programButterflyTextureCompute.bindComputeUnbind(texture_width, texture_height);
  
    // --------------------------------------------------------
    /** CREATING FOURIER COMPONENT DX/DY/DZ TEXTURES */

    // bind read textures in fourier component compute shader
    glBindImageTexture(6, texture_tilde_h0k.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    programFourierComponentCompute.SetUniform1i("tilde_h0k", 6);

    glBindImageTexture(2, texture_tilde_h0minusk.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    programFourierComponentCompute.SetUniform1i("h0minusk", 2);

    // bind write textures in fourier component compute shader
    texture_fourier_component_dx = Texture(false, texture_width, texture_height);
    glBindImageTexture(3, texture_fourier_component_dx.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(7, texture_fourier_component_dx.getID());
    programRender.SetUniform1i("fourier_component_dx", 7);

    texture_fourier_component_dy = Texture(false, texture_width, texture_height);
    glBindImageTexture(4, texture_fourier_component_dy.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(8, texture_fourier_component_dy.getID());
    programRender.SetUniform1i("fourier_component_dy", 8);

    texture_fourier_component_dz = Texture(false, texture_width, texture_height);
    glBindImageTexture(5, texture_fourier_component_dz.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTextureUnit(9, texture_fourier_component_dz.getID());
    programRender.SetUniform1i("fourier_component_dz", 9);

    programFourierComponentCompute.SetUniform1f("time",fourier_comp_time);
    programFourierComponentCompute.SetUniform1i("N", fourier_comp_N);
    programFourierComponentCompute.SetUniform1i("L", fourier_comp_L);

    programFourierComponentCompute.bindComputeUnbind(texture_width, texture_height);
}

void setUpLibraries()
{
    // initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
    }

    // create window
    window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
    }

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwMakeContextCurrent(window);

    // initialize GLEW
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
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_RELEASE) return;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){}

/** reverses the bits of a given unsigned int */
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