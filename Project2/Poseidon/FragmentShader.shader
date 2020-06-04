#version 450 core

in vec2 texcoord;

uniform sampler2D tilde_h0k;
uniform sampler2D h0minusk;
uniform sampler2D butterfly_texture;
uniform sampler2D fourier_component_dx;
uniform sampler2D fourier_component_dy;
uniform sampler2D fourier_component_dz;

out layout (location = 0) vec4 out_color;

void main() {
    //out_color = texture(tilde_h0k, texcoord) + texture(h0minusk, texcoord);
    //out_color = texture(butterfly_texture, texcoord);
    out_color = texture(fourier_component_dy, texcoord);
}