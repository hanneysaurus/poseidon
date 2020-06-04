#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 16) in;

//layout(binding = 7, rgba32f) writeonly uniform image2D fourier_component_dx; // left, right
//layout(binding = 8, rgba32f) writeonly uniform image2D fourier_component_dy; // front, back
layout(binding = 6, rgba32f) writeonly uniform image2D fourier_component_dz; // up, down direction (along height of wave)

layout(binding = 0, rgba32f) readonly uniform image2D tilde_h0k;
layout(binding = 1, rgba32f) readonly uniform image2D tilde_h0minusk;

uniform int N;
uniform int L;
uniform float time;

void main()
{
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
	imageStore(fourier_component_dz, texel, vec4(0, 1, 0, 1));
}