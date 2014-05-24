#version 330 core

in vec3 position;
out vec3 vPosition;

void main() {
	gl_Position = vec4(position, 1.0);
	vPosition = (position + 1.0)/2.0;
}