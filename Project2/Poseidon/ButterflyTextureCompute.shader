#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D butterfly_texture;

// assigns the contents of the reverseIndicesBuffer to an array. binding index = 0, as defined in main
layout(std430, binding = 0)  buffer reverseIndicesBuffer
{
	int reverseIndices[]; 
};

struct complexNumber
{
	float real;
	float imaginary;
};

void main()
{
	int N = 100;

	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	// red
	vec4 color= vec4(1.0,0,0,1);

	// test to see if it works (using texture height = 64) -> change color from red to green if it works!
	if (reverseIndices[1] == 32) {
		color = vec4(0.0, 1.0, 0.0, 1);
	}

	imageStore(butterfly_texture, texel, color);

}