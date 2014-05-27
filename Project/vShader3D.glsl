#version 330 core

layout(location=0) in vec3 position;
uniform sampler3D noise;
uniform mat4 model_view;
uniform mat4 projection;
uniform float tileSize;
uniform float radius;
uniform vec3 light_pos;

out vec3 vlight_dir;
out vec3 view_dir;
out vec3 normal_mv;
out vec3 vPosition;
out vec3 fPosition;
out vec3 uNormal;

float height(vec3 t) {
	return max(texture(noise, (t+radius) / (2.0*radius)).r, 0) / 5.0;
}

void main() {

	fPosition = position + height(position)*normalize(position);

	vec3 kPosition = position + height(position)*normalize(position);

	vPosition = (model_view * vec4(kPosition, 1.0)).xyz;

	gl_Position = projection * vec4(vPosition, 1.0);
	
	
	float h = 0.001;
	vec3 off = vec3(h, h, 0.0);
	vec3 N;
	
	float dx = ( height(position + vec3(off.xz, 0.0)) - height(position - vec3(off.xz, 0.0)) ) / (2*h);
	vec3 gdx = vec3(1.0, 0.0, dx);
	float dy = ( height(position + vec3(off.zx, 0.0)) - height(position - vec3(off.zx, 0.0)) ) / (2*h);
	vec3 gdy = vec3(0.0, 1.0, dy);
	N = normalize(cross(gdx, gdy));
	normal_mv = N;
	uNormal = N;

	float cosTh = normalize(position).z;
	float sinTh = sqrt(1-cosTh*cosTh);

	//We need to perform a roatation of angle theta around the axis P cross z.
	//P cross z = vec3(P.y, -P.x, 0.0));

	vec3 Pxz = normalize(vec3(position.y, -position.x, 0.0));
	//Now we build the rotation matrix around this axis.

	vec3 R1 = vec3(cosTh + Pxz.x*Pxz.x*(1-cosTh), Pxz.x*Pxz.y*(1-cosTh), Pxz.y * sinTh);
	vec3 R2 = vec3(Pxz.y*Pxz.x*(1-cosTh), cosTh + Pxz.y*Pxz.y*(1-cosTh), -Pxz.x*sinTh);
	vec3 R3 = vec3(-Pxz.y*sinTh, Pxz.x*sinTh, cosTh);

	mat3 rot1 = mat3(R1, R2, R3);

	normal_mv = rot1 * normal_mv;
	normal_mv = normalize ( mat3 ( transpose ( inverse ( model_view ) ) ) * normalize(normal_mv));



	fPosition = position;

	///fixed camera (specified in camera-land)
		///vlight_dir = -vPosition;
	///moving camera (render image is still)
		vlight_dir = ( model_view * vec4(light_pos,1.0)).xyz - vPosition.xyz;
	view_dir = -vec3(gl_Position);
}