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
int leftRotate(int n, unsigned int d);

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
unsigned int VAO, VBO;

// textures
unsigned int texture_read = 0;
unsigned int texture_tilde_h0k = 0;
unsigned int texture_tilde_h0minusk = 0;

unsigned int texture_random_noise_1 = 0;
unsigned int texture_random_noise_2 = 0;
unsigned int texture_random_noise_3 = 0;
unsigned int texture_random_noise_4 = 0;

unsigned int texture_butterfly = 0;

const unsigned int texture_width = 64;
const unsigned int texture_height = 64;

int* bitReversedIndices;



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
    glGenTextures(1, &texture_butterfly);

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

            random_noise_1_data[i][j][0] = rand_value; // RED
            random_noise_1_data[i][j][1] = rand_value; // GREEN 
            random_noise_1_data[i][j][2] = rand_value; // BLUE
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
    glBindTextureUnit(1, texture_tilde_h0k);
    int location = glGetUniformLocation(programRender.getID(), "tex1");
    glProgramUniform1i(programRender.getID(), location, 1);

    glBindTextureUnit(2, texture_tilde_h0minusk);
    location = glGetUniformLocation(programRender.getID(), "tex2");
    glProgramUniform1i(programRender.getID(), location, 2);

    glBindImageTexture(3, texture_random_noise_1, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex1");
    glProgramUniform1i(programTildeHCompute.getID(), location, 3);

    glBindImageTexture(4, texture_random_noise_2, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex2");
    glProgramUniform1i(programTildeHCompute.getID(), location, 4);

    glBindImageTexture(5, texture_random_noise_3, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex3");
    glProgramUniform1i(programTildeHCompute.getID(), location, 5);

    glBindImageTexture(6, texture_random_noise_4, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(programTildeHCompute.getID(), "randtex4");
    glProgramUniform1i(programTildeHCompute.getID(), location, 6);


    //invoke compute shader
    programTildeHCompute.bind();
    programTildeHCompute.dispatchCompute(texture_width, texture_height, 1);
    programTildeHCompute.unbind();

    /** BUTTERFLY COMPUTE SHADER */

    // define texture
    glBindTexture(GL_TEXTURE_2D, texture_butterfly);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, texture_width, texture_height);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindImageTexture(0, texture_butterfly, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // attach butterfly texture as uniform image to write to 
    glBindTextureUnit(0, texture_butterfly);
    location = glGetUniformLocation(programRender.getID(), "butterfly_texture");
    glProgramUniform1i(programRender.getID(), location, 0);


    bitReversedIndices = new int[texture_height];
    int bits = (log(texture_height) / log(2));
    for (int i = 0; i < texture_height; i++)
    {   
        // x is the new index that should be stored
        unsigned int x = reverseBits(i);
        // rotate left
        x = (x << bits) | (x >> (INT_SIZE - bits));
         
        //temporary debug
        std::cout << i << " " << x << std::endl;
        bitReversedIndices[i] = x;

    }

    // Create the buffer that passes bitReversedIndices to butterfly compute shader
    unsigned int reverseIndicesSSBO;
    glGenBuffers(1, &reverseIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reverseIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int)*texture_height, bitReversedIndices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, reverseIndicesSSBO); // buffer assigned to binding index 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind the buffer

    programButterflyTextureCompute.bind();
    programButterflyTextureCompute.dispatchCompute(texture_width, texture_height, 1);
    programButterflyTextureCompute.unbind();


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

    glDeleteTextures(1, &texture_read);
    glDeleteTextures(1, &texture_tilde_h0k);
    glDeleteTextures(1, &texture_tilde_h0minusk);

    glDeleteTextures(1, &texture_butterfly);

    glDeleteTextures(1, &texture_random_noise_1);
    glDeleteTextures(1, &texture_random_noise_2);
    glDeleteTextures(1, &texture_random_noise_3);
    glDeleteTextures(1, &texture_random_noise_4);

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


int leftRotate(int n, unsigned int d)
{

    /* In n<<d, last d bits are 0. To
     put first 3 bits of n at
    last, do bitwise or of n<<d
    with n >>(INT_BITS - d) */
    return (n << d) | (n >> (32 - d));

}



/**

private int[] initBitReversedIndices()
    {
        int[] bitReversedIndices = new int[N];
        int bits = (int) (Math.log(N)/Math.log(2));

        for (int i = 0; i<N; i++)
        {
            int x = Integer.reverse(i);
            x = Integer.rotateLeft(x, bits);
            bitReversedIndices[i] = x;
        }

        return bitReversedIndices;
    }
    */