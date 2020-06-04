#version 450 core
#define PI 3.1415
#define GRAVITY 9.81

layout(local_size_x = 1, local_size_y = 16) in;


// read textures
layout(binding = 0, rgba32f) readonly uniform image2D tilde_h0k;
layout(binding = 1, rgba32f) readonly uniform image2D h0minusk;

// write textures
layout(binding = 3, rgba32f)  writeonly uniform image2D fourier_component_dx;
layout(binding = 4, rgba32f)  writeonly uniform image2D fourier_component_dy;
layout(binding = 5, rgba32f)  writeonly uniform image2D fourier_component_dz;


uniform int N;
uniform int L;
uniform float time;

struct complex {
	float real;
	float im;
};

complex multiply(complex c0, complex c1) {
	complex product;
	product.real = c0.real * c1.real - c0.im * c1.im;
	product.im = c0.real * c1.im + c0.im * c1.real;
	return product;
};

complex add(complex c0, complex c1) {
	complex sum;
	sum.real = c0.real + c1.real;
	sum.im = c0.im + c1.im;
	return sum;
}

complex conjugate(complex c) {
	complex conjugate_of_c = complex(c.real, -c.im);
	return conjugate_of_c;
}


void main()
{

	ivec2 curr_pos = ivec2(gl_GlobalInvocationID.xy);

	vec2 x = ivec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	vec2 k = vec2(2.0 * PI * x.x / L, 2.0 * PI * x.y / L);

	float magnitude_of_k = length(k);
	if (magnitude_of_k < 0.00001) magnitude_of_k = 0.00001;

	float w = sqrt(GRAVITY * magnitude_of_k);

	vec2 tilde_h0k_values = imageLoad(tilde_h0k, curr_pos).rg;
	complex fourier_component = complex(tilde_h0k_values.x, tilde_h0k_values.y);

	vec2 tilde_h0minusk_values = imageLoad(h0minusk, curr_pos).rg;
	complex fourier_component_conj = conjugate(complex(tilde_h0minusk_values.x, tilde_h0minusk_values.y));

	float cos_w_t = cos(w * time);
	float sin_w_t = sin(w * time);

	// using euler e^(iwt) = cos(wt) + i*sin(wt)
	complex exp_iwt = complex(cos_w_t, sin_w_t);
	complex exp_iwt_inv = complex(cos_w_t, -sin_w_t);

	// find projections of spectrum onto each axis 

	// dy
	complex hkt_dy = add(multiply(fourier_component, exp_iwt), multiply(fourier_component_conj, exp_iwt_inv));

	// dx 
	complex dx = complex(0.0, -k.x / magnitude_of_k);
	complex hkt_dx = multiply(dx, hkt_dy);

	// dz 
	complex dy = complex(0.0, -k.y / magnitude_of_k);
	complex hkt_dz = multiply(dy, hkt_dy);

	// store components in textures
	imageStore(fourier_component_dy, curr_pos, vec4(hkt_dy.real, hkt_dy.im,0,1));
	imageStore(fourier_component_dx, curr_pos, vec4(hkt_dx.real, hkt_dx.im,0,1));
	imageStore(fourier_component_dz, curr_pos, vec4(hkt_dz.real, hkt_dz.im,0,1));

	// debugging 
	/*vec4 colour=imageLoad(tilde_h0k,curr_pos);
	imageStore(fourier_component_dy, curr_pos, colour);*/
	//if (N==256) imageStore(fourier_component_dy, curr_pos, vec4(0, 1, 0, 1));

}