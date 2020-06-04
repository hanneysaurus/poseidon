#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 16) in;

layout(binding = 3, rgba32f) uniform writeonly image2D fourier_component_dx; // left, right
layout(binding = 4, rgba32f) uniform writeonly image2D fourier_component_dy; // front, back
layout(binding = 5, rgba32f) uniform writeonly image2D fourier_component_dz; // up, down direction (along height of wave)

uniform int N;
uniform int L;
uniform float time;

void main()
{
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
	imageStore(fourier_component_dy, texel, vec4(0, 1, 0, 1));
}