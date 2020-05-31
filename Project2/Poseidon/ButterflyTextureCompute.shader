#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D butterfly_texture;



struct complexNumber
{
	float real;
	float imaginary;
};

void main()
{
	int N = 100;

	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	vec4 color= vec4(1.0,0,0,1);

	imageStore(butterfly_texture, texel, color);

}