#define WIN32_LEAN_AND_MEAN
#define STB_IMAGE_IMPLEMENTATION
#define INT_SIZE 32

#include <iostream>
#include <bitset> 

#include "OpenGLImports.h"

#include "ShaderProgram.h"
#include "Texture.h"

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
void init_VAO_and_shaders();
void init_textures();

int main(void);

// fft functions
void create_h0k_h0minusk_textures();
void create_butterfly_texture();
void create_fourier_components();
void fft();
void inversion();

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
ShaderProgram programButterflyCompute;
ShaderProgram programInversionCompute;
unsigned int VAO, VBO;


//textures
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

Texture texture_pingpong_0;
Texture texture_pingpong_1;

Texture texture_displacement_of_points_on_grid;

// uniform variables
float t = float(glfwGetTime());

// fft ocean parameters
float A = 4;
int L = 1000;
glm::vec2 windDirection = glm::vec2(1.0f, 1.0f);
float windSpeed = 40;

// width and height of grid
const int N = 256;
const int M = 256;

//misc
int* bitReversedIndices;
const int log_2_N = (int)(log(N)/log(2));
int pingpong;
float deltaTime = 0.0f;
float time_sum = 0.0f;

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

    init_VAO_and_shaders();
    init_textures();

    create_h0k_h0minusk_textures();
    create_butterfly_texture();
    create_fourier_components();
    fft();
    inversion();


    // CONNECT TO FRAGMENT SHADER
    // bind the resulting textures to fragment shader
    glBindTextureUnit(0, texture_tilde_h0k.getID());
    glBindTextureUnit(1, texture_tilde_h0minusk.getID());

    // bind resulting butterfly texture to fragment shader
    glBindTextureUnit(2, texture_butterfly.getID());

    // bind resulting dx, dy, dz fourier component textures  to fragment shader
    glBindTextureUnit(3, texture_fourier_component_dx.getID());
    glBindTextureUnit(4, texture_fourier_component_dy.getID());
    glBindTextureUnit(5, texture_fourier_component_dz.getID());

    // bind ping pong textures to fragment shader
    glBindTextureUnit(6, texture_displacement_of_points_on_grid.getID());
    glBindTextureUnit(7, texture_pingpong_1.getID());

    //connect displacement texture to the fragment shader
    //glBindTextureUnit(8, texture_displacement_of_points_on_grid.getID());

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

void init_VAO_and_shaders()
{
    // create shaders
    programRender = ShaderProgram("VertexShader.shader", "FragmentShader.shader");
    programTildeHCompute = ShaderProgram("tildehcompute.shader");
    programButterflyTextureCompute = ShaderProgram("butterflyTextureCompute.shader");
    programFourierComponentCompute = ShaderProgram("fourierComponentCompute.shader");
    programButterflyCompute = ShaderProgram("butterflyCompute.shader");
    programInversionCompute = ShaderProgram("inversionCompute.shader");

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

}

void init_textures()
{
    texture_tilde_h0k = Texture(false, N, M);
    texture_tilde_h0minusk = Texture(false, N, M);
    texture_random_noise_1 = Texture(true, N, M);
    texture_random_noise_2 = Texture(true, N, M);
    texture_random_noise_3 = Texture(true, N, M);
    texture_random_noise_4 = Texture(true, N, M);

    texture_butterfly = Texture(false, log(N) / log(2), M);

    texture_fourier_component_dx = Texture(false, N, M);
    texture_fourier_component_dy = Texture(false, N, M);
    texture_fourier_component_dz = Texture(false, N, M);

    texture_pingpong_0 = Texture(false, N, M);
    texture_pingpong_1 = Texture(false, N, M);

    texture_displacement_of_points_on_grid = Texture(false, N, M);

}

void create_h0k_h0minusk_textures() {

    // bind tilde_h0k, h0minusk & noise textures 
    // to image units which are then used by uniform image2Ds in tilde_hk compute shader */
    glBindImageTexture(0, texture_tilde_h0k.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glBindImageTexture(1, texture_tilde_h0minusk.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glBindImageTexture(2, texture_random_noise_1.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);

    glBindImageTexture(3, texture_random_noise_2.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);

    glBindImageTexture(4, texture_random_noise_3.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);

    glBindImageTexture(5, texture_random_noise_4.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA8);

    programTildeHCompute.SetUniform1i("N", N);
    programTildeHCompute.SetUniform1i("L", L);
    programTildeHCompute.SetUniform1f("A", A);
    programTildeHCompute.SetUniform1f("windSpeed", windSpeed);
    programTildeHCompute.SetUniform1fv("windDirection", windDirection);

    // run the tildeHCompute shader to write to textures
    programTildeHCompute.compute(N, M);

}

void create_butterfly_texture() {

    bitReversedIndices = new int[M];
    int bits = (log(M) / log(2));
    for (int i = 0; i < M; i++)
    {
        unsigned int x = reverseBits(i);
        x = (x << bits) | (x >> (INT_SIZE - bits));
        bitReversedIndices[i] = x;
    }

    // create the buffer that passes bitReversedIndices & bind to binding index
    unsigned int reverseIndicesSSBO;
    glGenBuffers(1, &reverseIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reverseIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * M, bitReversedIndices, GL_STATIC_DRAW);
    delete[] bitReversedIndices;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, reverseIndicesSSBO); // buffer assigned to binding index 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind the buffer after use

    // run the butterfly texture compute shader
    glBindImageTexture(1, texture_butterfly.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // run the butterfly compute shader to write to butterfly texture
    programButterflyTextureCompute.compute(N, M);
}

void create_fourier_components()
{
    // --------------------------------------------------------
    /** CREATING FOURIER COMPONENT DX/DY/DZ TEXTURES */

    // rebind the h0k and h0minusk to image unit 0 and 1, respectively.(a little unsure about this)
    glBindImageTexture(0, texture_tilde_h0k.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_tilde_h0minusk.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

    // bind image units used  in fourier component compute shader to dx dy dz write textures
    glBindImageTexture(2, texture_fourier_component_dx.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glBindImageTexture(3, texture_fourier_component_dy.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glBindImageTexture(4, texture_fourier_component_dz.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    programFourierComponentCompute.SetUniform1f("time", t);
    programFourierComponentCompute.SetUniform1i("N", N);
    programFourierComponentCompute.SetUniform1i("L", L);

    // run the programFourierComponent compute shader to write to dx, dy, dz textures
    programFourierComponentCompute.compute(N, M);
}

void fft()
{
    // --------------------------------------------------------
    /**  BUTTERFLY FOURIER COMPUTATION */

    glBindImageTexture(0, texture_butterfly.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

    glBindImageTexture(1, texture_fourier_component_dy.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glBindImageTexture(2, texture_pingpong_1.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    pingpong = 0;

    // 1 dim FFT in horizontal direction 
    for (int stage = 0; stage < log_2_N; stage++)
    {
        programButterflyCompute.updateButterflyComputeUniforms(pingpong, 0, stage);
        programButterflyCompute.compute(N, M);
        glFinish();
        pingpong = !pingpong;
    }

    // 1 dim FFT in vertical direction  
    for (int stage = 0; stage < log_2_N; stage++)
    {
        programButterflyCompute.updateButterflyComputeUniforms(pingpong, 1, stage);
        programButterflyCompute.compute(N, M);
        glFinish();
        pingpong = !pingpong;
    }
}

void inversion()
{

    // --------------------------------------------------------
    ///** INVERSION COMPUTE SHADER */

    glBindImageTexture(0, texture_displacement_of_points_on_grid.getID(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texture_fourier_component_dy.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(2, texture_pingpong_1.getID(), 0, false, 0, GL_READ_ONLY, GL_RGBA32F);

    programInversionCompute.SetUniform1i("pingpong", pingpong);
    programInversionCompute.SetUniform1i("N", N);

    programInversionCompute.compute(N, M);
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
    glClearColor(0.0f, 0.0f, 0.0f, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    t+=0.5;
    create_fourier_components();
    fft();
    inversion();


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
    glDeleteProgram(programFourierComponentCompute.getID());
    glDeleteProgram(programButterflyCompute.getID());
    glDeleteProgram(programInversionCompute.getID());

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    texture_random_noise_1.deleteTexture();
    texture_random_noise_2.deleteTexture();
    texture_random_noise_3.deleteTexture();
    texture_random_noise_4.deleteTexture();

    texture_tilde_h0k.deleteTexture();
    texture_tilde_h0minusk.deleteTexture();
    texture_butterfly.deleteTexture();

    texture_pingpong_0.deleteTexture();
    texture_pingpong_1.deleteTexture();

    texture_displacement_of_points_on_grid.deleteTexture();
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