#version 430 core

out vec4 FragColor;

uniform vec4 ourColor;

void main()
{
	FragColor = ourColor;
}