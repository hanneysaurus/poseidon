#pragma once

#include "stb_image.h"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <gl/glew.h>

class Texture
{
public:
	Texture();
	Texture(const char* textureFilePath);
	~Texture();

	unsigned int get();

	void use();

private:
	int width;
	int height;
	int nrChannels;

	unsigned char* data;
	unsigned int texture;
};


