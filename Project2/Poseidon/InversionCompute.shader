#version 450 core

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) writeonly uniform image2D displacement;

layout(binding = 1, rgba32f) readonly uniform image2D pingpong_0;
layout(binding = 2, rgba32f) readonly uniform image2D pingpong_1;

uniform int pingpong;
uniform int N;

void main(void)
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	float perms[] = { 1.0 ,-1.0 };
	int index = int(mod((int(x.x + x.y)), 2));
	float perm = perms[index];

	float NxN = (float) N * N;

	if(pingpong == 0)
	{
		float h = imageLoad(pingpong_0, x).r;
		imageStore(displacement, x, vec4(perm*(h / NxN), perm*(h / NxN), perm*(h / NxN),1));
	}
	else if(pingpong == 1)
	{
		float h = imageLoad(pingpong_1, x).r;
		imageStore(displacement, x, vec4(perm*(h/NxN), perm*(h/NxN), perm*(h/NxN),1));
	}
}