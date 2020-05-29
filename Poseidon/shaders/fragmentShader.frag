#version 450 core

out vec4 FragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

in vec2 texcoord;

void main()
{
    FragColor = texture(tex1, texcoord);   
}  