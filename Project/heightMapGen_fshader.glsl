#version 330 core

out vec3 color;
in vec3 vPosition;
out vec2 fPosition;

uniform sampler1D permTableTexture;
uniform sampler1D gradTableTexture;
uniform sampler1D expTableTexture;
uniform float Octaves;
uniform float Lacunarity;
uniform float H;

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

float exp(float t) {
	return texture(expTableTexture, t/256).r;
}

float perlin(float paramX, float paramY) {
	vec2 P = fPosition.xy * vec2(paramX, paramY);

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

	return f;
}

float fBm() {
	float value = 0.0;
	float i = 0.0;
	float remainder;
	for(i=0.0; i<Octaves; i++) {
		value += perlin(5,5) * exp(i);
		fPosition *= Lacunarity;
	}
	remainder = Octaves - floor(Octaves);
	if(remainder > 0.0) {
		value += remainder * perlin(5,5) * exp(i);
	}
	return value;
}

void main() {
	fPosition = vPosition.xy / 2 + vec2(0.5,0.5);
	float f = fBm();
	color = vec3(f);
}

