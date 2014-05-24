#version 330 core

layout(location=0) out vec3 color;
uniform sampler2D noiseTexture;
uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;
uniform sampler2DShadow shadowMap;

in vec3 fPosition;
in vec3 depthPos;
in vec3 vlight_dir;
in vec3 view_dir;
in vec3 normal_mv;
in vec3 uNormal;

vec3 sand() {
	return texture(sandTexture, 60*fPosition.xy).rgb;
}

vec3 rock() {
	return texture(rockTexture, 10*fPosition.xy).rgb;
}

vec3 grass() {
	return texture(grassTexture, 10*fPosition.xy).rgb;
}

vec3 snow() {
	return texture(snowTexture, 30*fPosition.xy).rgb;
}

float factor(float p) {
	return p*p*p*p*p;
}

vec3 colorZ(float z) {
	vec3 kd;
	float n = abs(normalize(uNormal).z);
	float rockF = factor(1-n), F = factor(n);
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

float shadow(vec3 a) {
	return texture(shadowMap, a / 2.0 + 0.5);
}

void main() {
	
	float visibility = 1.0;

	if( shadow(depthPos) < depthPos.z) {
		visibility = 0.5;
	}

	float z = max(texture(noiseTexture, fPosition.xy / 2.0 + 0.5).r, 0);

	vec3 kd = vec3(0.4,0.4,0.4);
	kd = colorZ(z);

	vec3 Id = vec3(1.0,1.0,1.0);
	vec3 I = Id * kd;/// * dot(normalize(normal_mv), normalize(vlight_dir));
	I = I;
	color = vec3(shadow(depthPos));/// vlight_dir; ///I;///texture(rockTexture, fPosition.xy / 2.0 + 0.5).rgb; ///I; /// vec3(texture(noise, fPosition.xy / 2.0 + 0.5).r);
}