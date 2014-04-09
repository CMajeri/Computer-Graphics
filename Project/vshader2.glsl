#version 330 core

layout(location=0) in vec3 position;
uniform sampler3D noise;
uniform mat4 model_view;
uniform mat4 projection;
uniform float tileSize;

uniform vec3 light_pos;

out vec3 vlight_dir;
out vec3 view_dir;
out vec3 normal_mv;
out vec3 vPosition;
out vec3 fPosition;

float height(vec3 t) {
	return max(texture(noise, (t+tileSize) / (2.0*tileSize)).r, 0);
}

void main() {

	fPosition = position;/// + height(position)*normalize(position);

	vPosition = (model_view * vec4(fPosition, 1.0)).xyz;

	gl_Position = projection * vec4(vPosition, 1.0);
	
	
	float h = 0.001;
	vec3 off = vec3(h, h, 0.0);
	vec3 N;
	
	float dx = ( height(position + vec3(off.xz, 0.0)) - height(position - vec3(off.xz, 0.0)) ) / (2*h);
	vec3 gdx = vec3(1.0, 0.0, dx);
	float dy = ( height(position + vec3(off.xz, 0.0)) - height(position - vec3(off.xz, 0.0)) ) / (2*h);
	vec3 gdy = vec3(0.0, 1.0, dy);
	N = normalize(cross(gdx, gdy));
	normal_mv = normalize ( mat3 ( inverse ( transpose ( model_view ) ) ) * N);

	float cosTh = position.z;
	float sinTh = sqrt(1 - cosTh*cosTh);
	float cosPh = normalize(position.xz).y;
	float sinPh = sqrt(1 - cosPh*cosPh);

	mat3 rot1 = mat3(
		vec3(cosTh, sinTh, 0),
		vec3(-sinTh, cosTh, 0),
		vec3(0, 0, -1)
	);

	mat3 rot2 = mat3(
		vec3(cosPh, 0, sinPh),
		vec3(-sinPh, 1, cosPh),
		vec3(0, 0, 0)
	);

	normal_mv = normal_mv;

	fPosition = position;

	vlight_dir = ( model_view * vec4(light_pos,1.0)).xyz - vPosition.xyz;
	view_dir = -vec3(gl_Position);
}