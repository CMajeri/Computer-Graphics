#version 330 core

layout(location=0) in vec3 position;
uniform sampler2D noise;
uniform mat4 model_view;
uniform mat4 projection;
uniform float tileSize;

uniform vec3 light_pos;

out vec3 vlight_dir;
out vec3 view_dir;
out vec3 normal_mv;
out vec3 vPosition;
out vec3 fPosition;

float height(vec2 t) {
	return max(texture(noise, (t+tileSize) / (2.0*tileSize)).r, 0);
}

void main() {

	fPosition = position + vec3(0.0,0.0,height(position.xy));

	vPosition = (model_view * vec4(position + vec3(0.0, 0.0, height(position.xy) ), 1.0)).xyz;
	gl_Position = projection * vec4(vPosition, 1.0);



	float h = 0.001;
	vec3 off = vec3(h, h, 0.0);
	vec3 N;
	
	float dx = ( height(position.xy + off.xz) - height(position.xy - off.xz) ) / (2*h);
	vec3 gdx = vec3(1.0, 0.0, dx);
	float dy = ( height(position.xy + off.zy) - height(position.xy - off.zy) ) / (2*h);
	vec3 gdy = vec3(0.0, 1.0, dy);
	N = normalize(cross(gdx, gdy));
	normal_mv = normalize ( mat3 ( inverse ( transpose ( model_view ) ) ) * N);

	vlight_dir = ( model_view * vec4(light_pos,1.0)).xyz - vPosition.xyz;
	view_dir = -vec3(gl_Position);
}