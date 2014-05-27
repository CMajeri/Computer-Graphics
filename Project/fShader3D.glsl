#version 330 core

#define PI 3.1415926535897932384626433832795
layout(location=0) out vec3 color;
uniform sampler3D noise;
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;
uniform float radius;

in vec3 fPosition;

in vec3 vlight_dir;
in vec3 view_dir;
in vec3 normal_mv;
in vec3 uNormal;


vec2 uv() {
	vec3 position = fPosition / radius;
	return vec2(0.5 + atan(position.y, position.x)/(2*PI), 0.5 - asin(position.z)/PI);
}

vec3 sand() {
	return texture(sandTexture, 60*uv()).rgb;
}

vec3 rock() {
	return texture(rockTexture, 10*uv()).rgb;
}

vec3 grass() {
	return texture(grassTexture, 10*uv()).rgb;
}

vec3 snow() {
	return texture(snowTexture, 30*uv()).rgb;
}

vec3 colorZ(float z) {
	vec3 kd;
	float n = abs(normalize(uNormal).z);
	if(z == 0.0) {
		kd = vec3(0.0,0.0,0.5);
	} else {
		if(z < 0.3) {
			kd = clamp(mix(clamp(mix(sand(), rock(), 1-n), 0, 1), clamp(mix(grass(), rock(), 1-n), 0, 1), z*3.333), 0, 1);
		} else {
			kd = clamp(mix(clamp(mix(grass(), rock(), 1-n), 0, 1), clamp(mix(grass(), snow(), 1-n), 0, 1), (z-0.3)*(1/0.7)), 0, 1);
		}
	}
	return kd;
}

void main() {

	float z = max( texture(noise, (fPosition+radius) / (2.0*radius)).r, 0 ) * 10.0;

	vec3 kd = vec3(0.4,0.4,0.4);
	kd = colorZ(z);

	vec3 Id = vec3(1.0,1.0,1.0);
	vec3 I = Id * kd * dot(normalize(normal_mv), normalize(vlight_dir));
	I = I;
	
	color =  I;///vec3(texture(noise, fPosition / 2.0 + 0.5).r);///I; ///vec3(0.0,0.0,0.0); ///I; ///
}