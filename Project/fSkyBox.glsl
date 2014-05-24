#version 330 core

in vec3 vPosition;
uniform samplerCube skyTexture;
uniform int layer;

out vec3 color;

void main() {
	color = texture(skyTexture, vPosition).rgb;
}