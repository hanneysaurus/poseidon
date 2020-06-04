#version 450 core
#define PI 3.1415

layout(local_size_x = 1, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D butterfly_texture;

// assigns the contents of the reverseIndicesBuffer to an array. binding index = 0, as defined in main
layout(std430, binding = 0)  buffer reverseIndicesBuffer
{
	int reverseIndices[]; 
} bit_reversed;

struct ComplexNumber
{
	float real;
	float imaginary;
};

void main()
{
	int N = 256;
	
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	float k = mod(x.y * (float(N) / pow(2, x.x + 1)), N);
	ComplexNumber twiddle = ComplexNumber(cos((2.0 * PI * k) / float(N)), sin((2.0 * PI * k) / float(N)));
	
	int butterflyspan = int(pow(2, x.x));

	int butterflywing = 0;

	if (mod(x.y, pow(2, x.x + 1)) < pow(2, x.x))
	{
		butterflywing = 1;
	}

	if (x.x == 0)
	{
		if (butterflywing == 1)
		{
			imageStore(butterfly_texture, x, vec4(twiddle.real, twiddle.imaginary, bit_reversed.reverseIndices[x.y], bit_reversed.reverseIndices[x.y + 1]));
		}
		else
		{
			imageStore(butterfly_texture, x, vec4(twiddle.real, twiddle.imaginary, bit_reversed.reverseIndices[x.y - 1], bit_reversed.reverseIndices[x.y]));
		}
	}
	else
	{
		if (butterflywing == 1)
		{
			imageStore(butterfly_texture, x, vec4(twiddle.real, twiddle.imaginary, x.y, x.y + 1));
		}
		else
		{
			imageStore(butterfly_texture, x, vec4(twiddle.real, twiddle.imaginary, x.y - butterflyspan, x.y));
		}
	}

	/*vec4 color = vec4(1.0,0,0,1);

	// test to see if it works (using texture height = 64) -> change color from red to green if it works!
	if (reverseIndices[1] == 32) {
		color = vec4(0.0, 1.0, 0.0, 1);
	}

	imageStore(butterfly_texture, texel, color);*/

}