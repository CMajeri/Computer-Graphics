#version 330 core

layout(location=0) in vec3 position;

uniform sampler2D noiseTexture;
uniform mat4 model_view;
uniform mat4 projection;
uniform float tileSize;
uniform vec3 light_pos;
uniform mat4 dMVP;

out vec3 vlight_dir;
out vec3 view_dir;
out vec3 normal_mv;
out vec3 vPosition;
out vec3 fPosition;
out vec3 uNormal;
out vec3 depthPos;

float height(vec2 t) {
	return max(texture(noiseTexture, (t + 1) / 2.0).r, 0) / 5.0;
}

void main() {

	fPosition = position + height(position.xy)*vec3(0.0, 0.0, 1.0);

	vPosition = (model_view * vec4(fPosition, 1.0)).xyz;

	gl_Position = projection * vec4(vPosition, 1.0);
	
	depthPos = (dMVP * vec4(fPosition, 1.0)).xyz;

	float h = 0.001;
	vec3 off = vec3(h, h, 0.0);
	vec3 N;
	
	float dx = ( height(position.xy + off.xz) - height(position.xy - off.xz) ) / (2*h);
	vec3 gdx = vec3(1.0, 0.0, dx);
	float dy = ( height(position.xy + off.zx) - height(position.xy - off.zx) ) / (2*h);
	vec3 gdy = vec3(0.0, 1.0, dy);

	N = normalize(cross(gdx, gdy));

	normal_mv = N;
	uNormal = N;
	normal_mv = normalize ( mat3 ( inverse ( transpose ( model_view ) ) ) * normalize(normal_mv));

	fPosition = position;

	///fixed camera (specified in camera-land)
		///vlight_dir = light_pos - vPosition;
	///moving camera (render image is still)
		vlight_dir = ( model_view * vec4(light_pos,1.0)).xyz - vPosition;

	view_dir = -vec3(gl_Position);
}