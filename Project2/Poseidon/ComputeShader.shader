#version 450 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform writeonly image2D tilde_h0k;

layout(binding = 1, rgba32f) uniform writeonly image2D tilde_h0minusk;

uniform readonly layout(rgba8) image2D randtex1;
uniform readonly layout(rgba8) image2D randtex2;
uniform readonly layout(rgba8) image2D randtex3;
uniform readonly layout(rgba8) image2D randtex4;

float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
	vec4 green = vec4(0.0, 1.0, 0.0, 1.0);

	vec4 noise_tex_color = imageLoad(randtex4, texel);

	imageStore(tilde_h0k, texel, noise_tex_color);
	imageStore(tilde_h0minusk, texel, green);
}