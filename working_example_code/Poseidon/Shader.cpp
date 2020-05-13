#include "Shader.h"
#include <iostream>
#include <fstream>
#include "gl/glew.h"


std::string LoadTextFile(const std::string& filepath)
{
    std::string result(""), line;
    std::fstream f(filepath, std::ios::in);
    while (f.good())
    {
        std::getline(f, line);
        result += line + '\n';
    }
    return result;
}


std::string ShaderTypeName(unsigned int shader)
{
    if (glIsShader(shader))
    {
        int type;
        glGetShaderiv(shader, GL_SHADER_TYPE, &type);

        if (type == GL_VERTEX_SHADER)
            return "Vertex Shader";
        if (type == GL_TESS_CONTROL_SHADER)
            return "Tessellation Control Shader";
        if (type == GL_TESS_EVALUATION_SHADER)
            return "Tessellation Evaluation Shader";
        if (type == GL_GEOMETRY_SHADER)
            return "Geometry Shader";
        if (type == GL_FRAGMENT_SHADER)
            return "Fragment Shader";
        if (type == GL_COMPUTE_SHADER)
            return "Compute Shader";
    }

    return "invalid shader";
}


bool CompileShader(unsigned int shader, const std::string& sourcecode)
{
    if (!glIsShader(shader))
    {
        std::cout << "ERROR: shader compilation failed, no valid shader specified" << std::endl;
        return false;
    }

    if (sourcecode.empty())
    {
        std::cout << "ERROR: shader compilation failed, no source code specified  (" << ShaderTypeName(shader) << ")" << std::endl;
        return false;
    }

    // array of source code components
    const char* sourcearray[] = { sourcecode.c_str() };

    // set source code
    glShaderSource(shader, 1, sourcearray, NULL);

    // compile shaders
    glCompileShader(shader);

    // check compile status
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    // successfully compiled shader
    if (status == GL_TRUE)
        return true;

    // show compile errors
    std::cout << "ERROR: shader compilation failed  (" << ShaderTypeName(shader) << ")" << std::endl << ShaderInfoLog(shader) << std::endl;

    return false;
}


std::string ShaderInfoLog(unsigned int shader)
{
    if (glIsShader(shader))
    {
        int logsize;
        char infolog[1024] = { 0 };
        glGetShaderInfoLog(shader, 1024, &logsize, infolog);

        return std::string(infolog);
    }

    return "invalid shader";
}


bool LinkProgram(unsigned int program, const std::list<unsigned int>& shaderlist)
{
    if (!glIsProgram(program))
    {
        std::cout << "ERROR: shader linking failed, no valid program specified" << std::endl;
        return false;
    }

    // attach all shaders to the program
    for (auto& shader : shaderlist)
    {
        if (glIsShader(shader))
            glAttachShader(program, shader);
    }

    // link program
    glLinkProgram(program);

    // detach all shaders again
    for (auto& shader : shaderlist)
    {
        if (glIsShader(shader))
            glDetachShader(program, shader);
    }

    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    // successfully linked program
    if (status == GL_TRUE)
        return true;

    // show link errors
    std::cout << "ERROR: shader linking failed" << std::endl << ProgramInfoLog(program) << std::endl;

    return false;
}


std::string ProgramInfoLog(unsigned int program)
{
    if (glIsProgram(program))
    {
        int logsize;
        char infolog[1024] = { 0 };
        glGetProgramInfoLog(program, 1024, &logsize, infolog);

        return std::string(infolog);
    }

    return "invalid program";
}