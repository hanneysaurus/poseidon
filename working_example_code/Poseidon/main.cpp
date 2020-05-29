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

using namespace std;
using namespace glm;


// main functions
void Initialize();
void Render();
void CleanUp();


GLFWwindow* window = nullptr;

unsigned int program_compute = 0;
unsigned int computeshader = 0;

unsigned int program_render = 0;
unsigned int vertexshader = 0;
unsigned int fragmentshader = 0;
unsigned int vertexarray = 0;
unsigned int vertexbuffer = 0;

unsigned int texture_read = 0;
unsigned int texture_draw = 0;
unsigned int texturewidth = 2;
unsigned int textureheight = 5;


string computeshader_source = {
    "#version 450 core\n"
    "layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;"
    "uniform readonly layout (rgba8) image2D img1;"
    "uniform writeonly image2D img2;"
    "void main() {"
    "ivec2 texel = ivec2(gl_GlobalInvocationID.xy);"
    "vec4 color = imageLoad(img1, texel);"
    "imageStore(img2, texel, color);"
    "}"
};

string vertexshader_source = {
    "#version 450 core\n"
    "in layout (location = 0) vec3 in_position;"
    "in layout (location = 1) vec2 in_texcoord;"
    "out vec2 texcoord;"
    "void main() {"
    "gl_Position = vec4(in_position, 1);"
    "texcoord = in_texcoord;"
    "}"
};

string fragmentshader_source = {
    "#version 450 core\n"
    "in vec2 texcoord;"
    "uniform sampler2D tex1;"
    "out layout (location = 0) vec4 out_color;"
    "void main() {"
    "out_color = texture(tex1, texcoord);"
    "}"
};


struct Vertex {
    vec3 Position;
    vec2 TexCoord;
};


int main(void)
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
    program_compute = glCreateProgram();
    program_render = glCreateProgram();
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    computeshader = glCreateShader(GL_COMPUTE_SHADER);
    glGenVertexArrays(1, &vertexarray);
    glGenBuffers(1, &vertexbuffer);
    glGenTextures(1, &texture_read);
    glGenTextures(1, &texture_draw);
    //------------------------------------------------------------------------------------------------------------------------


    // setup programs
    //------------------------------------------------------------------------------------------------------------------------
    // compute
    CompileShader(computeshader, computeshader_source);
    LinkProgram(program_compute, { computeshader });

    // render
    CompileShader(vertexshader, vertexshader_source);
    CompileShader(fragmentshader, fragmentshader_source);
    LinkProgram(program_render, { vertexshader, fragmentshader });
    //------------------------------------------------------------------------------------------------------------------------


    // setup vertex array
    //------------------------------------------------------------------------------------------------------------------------
    glBindVertexArray(vertexarray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*)(sizeof(vec3)));
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
        { { -0.9f,    -0.9f,    0 },{ 0, 0 } },
        { { +0.9f,    -0.9f,    0 },{ 1, 0 } },
        { { +0.9f,    +0.9f,    0 },{ 1, 1 } },
        { { -0.9f,    +0.9f,    0 },{ 0, 1 } },
    };
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //------------------------------------------------------------------------------------------------------------------------


    // setup textures ("read" and "draw")
    //------------------------------------------------------------------------------------------------------------------------
    // 2 x 5 texture data
    unsigned char texture_read_data[] = {
        0xFF, 0x00, 0x00, 0xFF,        0x00, 0xFF, 0x00, 0xFF,        // red            green
        0x00, 0x00, 0xFF, 0xFF,        0x00, 0xFF, 0xFF, 0xFF,        // blue            cyan
        0xFF, 0x00, 0xFF, 0xFF,        0xFF, 0xFF, 0x00, 0xFF,        // magenta    yellow
        0xFF, 0xFF, 0xFF, 0xFF,        0x00, 0x00, 0x00, 0xFF,        // white            black
        0x50, 0x50, 0x50, 0xFF,        0xA0, 0xA0, 0xA0, 0xFF,        // gray            gray (brighter)
    };

    glBindTexture(GL_TEXTURE_2D, texture_read);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texturewidth, textureheight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texturewidth, textureheight, GL_RGBA, GL_UNSIGNED_BYTE, texture_read_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, texture_draw);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texturewidth, textureheight);
    glBindTexture(GL_TEXTURE_2D, 0);

    // connect "texture_draw" to sampler in fragment shader
    unsigned int texture_unit = 1;
    glBindTextureUnit(texture_unit, texture_draw);
    int location = glGetUniformLocation(program_render, "tex1");
    glProgramUniform1i(program_render, location, texture_unit);

    // connect "texture_read" to image variable in compute shader
    unsigned int image_unit = 2;
    glBindImageTexture(image_unit, texture_read, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
    location = glGetUniformLocation(program_compute, "img1");
    glProgramUniform1i(program_compute, location, image_unit);

    // connect "texture_draw" to image variable in compute shader
    image_unit = 3;
    glBindImageTexture(image_unit, texture_draw, 0, false, 0, GL_WRITE_ONLY, GL_RGBA8);
    location = glGetUniformLocation(program_compute, "img2");
    glProgramUniform1i(program_compute, location, image_unit);
    //------------------------------------------------------------------------------------------------------------------------


    // invoke compute shader
    //------------------------------------------------------------------------------------------------------------------------
    glUseProgram(program_compute);
    glDispatchCompute(texturewidth, textureheight, 1);
    glUseProgram(0);
    /*
    invokes the compute shader ...
            texturewidth        x    textureheight    x    1
        =    2                        x    5                        x    1
        = 10 times (once per texel)
    each invocation can be identified by its "uvec3 gl_GlobalInvocationID"
    in this case:
        gl_GlobalInvocationID.x will be 0 ... 1
        gl_GlobalInvocationID.y will be 0 ... 4
        gl_GlobalInvocationID.z will be 0
        x and y will be used to access a certain texel in the texture
    */
    //------------------------------------------------------------------------------------------------------------------------

}


void Render()
{
    glClearColor(0.5f, 0.5f, 0.5f, 0);    // background = gray
    glClear(GL_COLOR_BUFFER_BIT);

    // render quad
    glUseProgram(program_render);
    glBindVertexArray(vertexarray);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}


void CleanUp()
{
    // delete all objects
    glDeleteProgram(program_compute);
    glDeleteProgram(program_render);
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    glDeleteShader(computeshader);
    glDeleteVertexArrays(1, &vertexarray);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteTextures(1, &texture_read);
    glDeleteTextures(1, &texture_draw);
}
