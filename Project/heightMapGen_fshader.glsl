#version 330 core

out vec3 color;
in vec3 vPosition;

uniform sampler1D permTableTexture;
uniform sampler1D gradTableTexture;

float permute(float x) {
	return texture(permTableTexture, x/256).r*256;
}

vec2 gradient(vec2 A) {
	return texture(gradTableTexture, permute( A.y + permute(A.x) ) / 256).rg;
}

float smoothDist(float x, float x0) {
	float k = x-x0;
	return (3 - 2*k)*k*k;
}

void main() {

	float paramX = 5;
	float paramY = 6;

	vec2 P = vPosition.xy / 2 + vec2(0.5,0.5);
	P = P * vec2(paramX, paramY);

	vec2 Fp = floor(P);

	vec2 A = vec2(Fp.x, Fp.y);
	float a = dot(gradient(A), P-A);

	vec2 B = vec2(Fp.x, Fp.y+1);
	float b = dot(gradient(B), P-B);
	
	vec2 C = vec2(Fp.x+1, Fp.y+1);
	float c = dot(gradient(C), P-C);

	vec2 D = vec2(Fp.x+1, Fp.y);
	float d = dot(gradient(D), P-D);
	
	float Sx = smoothDist(P.x, Fp.x);
	float AD = a + Sx*(d-a);
	float BC = b + Sx*(c-b);

	float Sy = smoothDist(P.y, Fp.y);

	float f = AD + Sy*(BC - AD) ;
	
	color = vec3(f,f,f);
}

