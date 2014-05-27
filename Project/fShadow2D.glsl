#version 330 core

in vec3 vPosition;
layout(location=0) out float color;

void main() {
	color = vPosition.z;
}