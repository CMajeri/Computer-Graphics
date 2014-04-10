#version 330 core

layout(location=0) out vec3 color;
uniform sampler3D noise;
in vec3 fPosition;

in vec3 vlight_dir;
in vec3 view_dir;
in vec3 normal_mv;

vec3 colorZ(float z) {
	vec3 kd;
	if(z < 0.0) {
		kd = vec3(0.4,0.4,0.4);
	} else {
		if(z < 0.02) {
			kd = vec3(0.5, 0.5, 0.0);
		} else {
			if(z < 0.04) {
				kd = vec3(0.1, 0.5, 0.0);
			} else {
				if(z < 0.06) {
					kd = vec3(0.4,0.4,0.4);
				} else {
					kd = vec3(0.8,0.8,0.8);
				}
			}
		}
	}
	return kd;
}

void main() {

	float z = max(texture(noise, fPosition / 2.0 + 0.5).r, 0);

	vec3 kd = vec3(0.4,0.4,0.4);
	kd = colorZ(z);

	vec3 Id = vec3(1.0,1.0,1.0);
	vec3 I = Id * kd * dot(normalize(normal_mv), normalize(vlight_dir));
	I = I;
	
	color = I; ///normalize(normal_mv); /// vec3(texture(noise, fPosition / 2.0 + 0.5).r) * 10.0;
}