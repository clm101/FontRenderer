#version 450

vec3 color = vec3(0.9, 0.9, 0.9);

layout(location = 0) in vec2 pos;
layout(location = 0) out vec3 fragColor;

void main(){
	gl_Position = vec4(pos, 0.0, 1.0);
	fragColor = color;
}