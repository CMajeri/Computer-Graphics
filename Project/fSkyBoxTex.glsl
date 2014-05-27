#version 330 core

in vec3 vPosition;
uniform sampler2D skyTex;
uniform int layer;

out vec3 color;

void main() {
	vec3 pp = vPosition;
	if(layer == 0) {
		pp = vec3(vPosition.y, 1 - vPosition.x, 0.0);
		vec2 p = vec2(pp.x/4.0  + 2/4.0, pp.y/3.0 + 1/3.0);
		color = texture(skyTex, p).rgb;
	}
	if(layer == 1) {
		pp = vec3(1-vPosition.y, vPosition.x, 0.0);
		vec2 p = vec2(pp.x/4.0, pp.y/3.0 + 1/3.0);
		color = texture(skyTex, p).rgb;
	}
	if(layer == 2) {
		vec2 p = vec2(pp.x/4.0 + 1/4.0, pp.y/3.0 + 1/3.0);
		color = texture(skyTex, p).rgb;
	}
	if(layer == 3) {
		pp = 1 - vPosition;
		vec2 p = vec2(pp.x/4.0 + 3/4.0, pp.y/3.0 + 1/3.0);
		color = texture(skyTex, p).rgb;
	}
	if(layer == 4) {
		vec2 p = vec2( vPosition.x/4.0 + 1/4.0, vPosition.y/3.0 + 2/3.0);
		color = texture(skyTex, p).rgb;
	}
	if(layer == 5) {
		vec2 p = vec2(vPosition.x/4.0 + 1/4.0, vPosition.y/3.0);
		color = texture(skyTex, p).rgb;	
	}

}