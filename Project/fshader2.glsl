#version 330 core

layout(location=0) out vec3 color;
uniform sampler2D noise;
in vec3 fPosition;

in vec3 vlight_dir;
in vec3 view_dir;
in vec3 normal_mv;

void main() {

	vec3 kd = vec3(0.4,0.4,0.4);
	float z = fPosition.z;
	if(z < 0.2) {
		kd = vec3(0.3,0.2,0.0);
	} else {
		if(z < 0.4) {
			kd = vec3(0.8, 0.8, 0.8);
		} else {
			kd = vec3(0.1,0.1,0.0);
		}
	}
	
	vec3 Id = vec3(1.0,1.0,1.0);

	vec3 I = Id * kd * dot(normalize(normal_mv), normalize(vlight_dir) );
	color = I;

}