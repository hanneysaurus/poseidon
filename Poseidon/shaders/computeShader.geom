#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 128) out;

in VS_OUT {
	vec3 color;
} gs_in[];

out vec3 fColor;

void main() {  
	
	vec4 currPos = gl_in[0].gl_Position;
	
	fColor = vec3(1.0, 0.0, 0.0);
	gl_Position = currPos + vec4(-0.1, -0.1, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(0.1, -0.1, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(-0.1, 0.1, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(0.1, 0.1, 0.0, 0.0);
    EmitVertex();
	
	fColor = vec3(0.0, 1.0, 0.0);
	gl_Position = currPos + vec4(-0.2, -0.2, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(0.2, -0.2, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(-0.2, 0.2, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(0.2, 0.2, 0.0, 0.0);
    EmitVertex();
	
	fColor = vec3(0.0, 0.0, 1.0);
	gl_Position = currPos + vec4(-0.3, -0.3, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(0.3, -0.3, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(-0.3, 0.3, 0.0, 0.0);
    EmitVertex();
	gl_Position = currPos + vec4(0.3, 0.3, 0.0, 0.0);
    EmitVertex();
    
	EndPrimitive();
}  