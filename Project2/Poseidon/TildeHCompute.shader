#version 450 core
#define PI 3.1415
#define GRAVITY 9.81

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

vec4 gauss_random(ivec2 co) {
	float noise1 = clamp(imageLoad(randtex1, co).r, 0.001, 1.0);
	float noise2 = clamp(imageLoad(randtex1, co).r, 0.001, 1.0);
	float noise3 = clamp(imageLoad(randtex1, co).r, 0.001, 1.0);
	float noise4 = clamp(imageLoad(randtex4, co).r, 0.001, 1.0);

	float u0 = 2.0 * PI * noise1;
	float v0 = sqrt(-2.0 * log(noise2));
	float u1 = 2.0 * PI * noise3;
	float v1 = sqrt(-2.0 * log(noise4));

	vec4 random = vec4(v0 * cos(u0), v0 * sin(u0), v1 * cos(u1), v1 * sin(u1));

	return random;
}

void main() {

	//should be uniforms later
	int N = 256;
	int L = 2048;
	float A = 4;
	vec2 windDirection = vec2(1.0f, 1.0f);
	float windSpeed = 40;

	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
	vec2 x = vec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;

	vec2 k = vec2(2.0 * PI * x.x / L, 2.0 * PI * x.y / L);

	float L_ = (windSpeed * windSpeed) / GRAVITY;
	float magnitude = length(k);
	if (magnitude < 0.00001) {
		magnitude = 0.00001;
	}
	float magnitude_squared = magnitude * magnitude;

	float h0k = clamp(sqrt((A / (magnitude_squared * magnitude_squared))
		* pow(dot(normalize(k), normalize(windDirection)), 6.0)
		* exp(-(1.0 / (magnitude_squared * L_ * L_)))
		* exp(-magnitude_squared * pow(L / 2000.0, 2.0))) / sqrt(2.0), -4000, 4000);

	float h0minusk = clamp(sqrt((A / (magnitude_squared * magnitude_squared))
		* pow(dot(normalize(-k), normalize(windDirection)), 6.0)
		* exp(-(1.0 / (magnitude_squared * L_ * L_)))
		* exp(-magnitude_squared * pow(L / 2000.0, 2.0))) / sqrt(2.0), -4000, 4000);

	vec4 gauss_randoms = gauss_random(texel);

	imageStore(tilde_h0k, texel, vec4(gauss_randoms.xy * h0k, 0, 1));
	imageStore(tilde_h0minusk, texel, vec4(gauss_randoms.zw * h0minusk, 0, 1));
}