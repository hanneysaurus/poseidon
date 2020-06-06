#version 450 core

in vec2 texcoord;

layout(binding = 0) uniform sampler2D tilde_h0k;
layout(binding = 1) uniform sampler2D h0minusk;
layout(binding = 2) uniform sampler2D butterfly_texture;
layout(binding = 3) uniform sampler2D fourier_component_dx;
layout(binding = 4) uniform sampler2D fourier_component_dy;
layout(binding = 5) uniform sampler2D fourier_component_dz;
layout(binding = 6) uniform sampler2D pingpong_0;
layout(binding = 7) uniform sampler2D pingpong_1;

out layout (location = 0) vec4 out_color;

void main() {
    //out_color = vec4(0, 1, 0, 1);
    out_color = texture(tilde_h0k, texcoord) - texture(h0minusk, texcoord);
    //out_color = texture(butterfly_texture, texcoord);
    //out_color = texture(fourier_component_dy, texcoord);
    //out_color = texture(pingpong_1, texcoord);
}