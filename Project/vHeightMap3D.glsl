#version 330 core

in vec3 position;
out vec3 vPosition;
uniform mat4 MVP;

void main() {
	gl_Position = vec4(position, 1.0);
	vPosition = position;
}