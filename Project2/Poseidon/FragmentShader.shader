#version 450 core

in vec2 texcoord;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform sampler2D butterfly_texture;
uniform sampler2D fourier_component_dx;
uniform sampler2D fourier_component_dy;
uniform sampler2D fourier_component_dz;

out layout(location = 0) vec4 out_color;

void main() {
    //out_color = texture(tex1, texcoord) + texture(tex2, texcoord);
    //out_color = texture(butterfly_texture, texcoord);
    out_color = texture(fourier_component_dz, texcoord);
}