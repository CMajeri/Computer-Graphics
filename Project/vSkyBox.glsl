#version 330 core

in vec3 position;
uniform mat4 MVPID;
out vec3 vPosition;


void main() {
	
	gl_Position = MVPID*vec4(position, 1.0);
	vPosition = position;
}