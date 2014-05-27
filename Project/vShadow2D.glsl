#version 330 core

layout(location = 0) in vec3 position;
uniform sampler2D noiseTexture;
uniform mat4 dMVP;
out vec3 vPosition;
float height(vec2 t) {
	return max(texture(noiseTexture, (t + 1) / 2.0).r, 0) / 5.0;
}
void main() {
	gl_Position = dMVP * vec4( position + height(position.xy) * vec3(0.0,0.0,1.0), 1.0 );
	vPosition = gl_Position.xyz;
}