#pragma once

#include <string>
#include <list>

// shader functions
std::string LoadTextFile(const std::string& filepath);
std::string ShaderTypeName(unsigned int shader);
bool CompileShader(unsigned int shader, const std::string& sourcecode);
std::string ShaderInfoLog(unsigned int shader);
bool LinkProgram(unsigned int program, const std::list<unsigned int>& shaderlist);
std::string ProgramInfoLog(unsigned int program);

