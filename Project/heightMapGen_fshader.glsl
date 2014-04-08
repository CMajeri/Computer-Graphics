#version 330 core

out vec3 color;
in vec3 vPosition;
out vec2 fPosition;

uniform sampler1D permTableTexture;
uniform sampler1D gradTableTexture;
uniform sampler1D expTableTexture;
uniform sampler1D poissonTable;
uniform float Octaves;
uniform float Lacunarity;
uniform float H;
uniform float paramX;
uniform float paramY;
uniform float tileSize;

uniform int nCubes;

float poisson(int t) {
	return texture(poissonTable, t/256.0).r;
}

void addPoint(in ivec2 cubeId, in vec2 at, inout float distVector[3], inout vec2 delta[3], inout int ID[3]) {

	vec2 d;
	vec2 f;
	float d2;
	int count;
	int i, j, index;
	int this_id;

	int seed = 702395077*cubeId.x + 915488749*cubeId.y;
		
	count = int(poisson(seed >> 24));

	seed=1402024253*seed+586950981;

	for(j = 0; j <= count; j++) {
		this_id = seed;

		f.x=(seed+0.5)*(1.0/4294967296.0);
		seed=1402024253*seed+586950981;

		f.y=(seed+0.5)*(1.0/4294967296.0);
		seed=1402024253*seed+586950981;

		d.x = cubeId.x + f.x - at.x;
		d.y = cubeId.y + f.y - at.y;

		d2 = length(d);

		if(d2 < distVector[2]) {
			index = 3;
			while (index > 0 && d2 < distVector[index-1]) index--;
			for(i = 2; i > index; i--) {
				distVector[i+1]=distVector[i];
				ID[i+1]=ID[i];
				delta[i+1]=delta[i];
			}
			distVector[index]=d2;
			ID[index]=this_id;
			delta[index]=d;
		}
	}
	return;
}

float Worley() {
	float distVector[3];
	float DENSITY_ADJUSTMENT = 0.398150;

	distVector[0] = 9999.9;
	distVector[1] = 9999.9;
	distVector[2] = 9999.9;


	vec2 delta[3];
	int ID[3];

	vec2 at = fPosition.xy*nCubes;
	vec2 new_at = at*DENSITY_ADJUSTMENT;

	ivec2 Fat;
	Fat.x = int(new_at.x);
	Fat.y = int(new_at.y);

	int ii, jj, id = 0, ie = 0, jd = 0, je = 0;
	if(Fat.x > 0) {
		id = -1;
	}
	if(Fat.x < nCubes - 1) {
		ie = 1;
	}
	if(Fat.y > 0) {
		jd = -1;
	}
	if(Fat.y < nCubes - 1) {
		je = 1;
	}
	for (ii = id; ii<=ie; ii++) {
		for (jj = jd; jj<=je; jj++) {
			addPoint(Fat + ivec2(ii,jj), new_at, distVector, delta, ID);
		}
	}

	distVector[0] = distVector[0]*(1.0/DENSITY_ADJUSTMENT);
	distVector[1] = distVector[1]*(1.0/DENSITY_ADJUSTMENT);
	distVector[2] = sqrt(distVector[2])*(1.0/DENSITY_ADJUSTMENT);
	delta[0] *= (1.0/DENSITY_ADJUSTMENT);
	delta[1] *= (1.0/DENSITY_ADJUSTMENT);
	delta[2] *= (1.0/DENSITY_ADJUSTMENT);

	return (distVector[1])/3;
}


float permute(float x) {
	return texture(permTableTexture, x/256).r;
}

vec2 gradient(vec2 A) {
	return texture(gradTableTexture, ( A.y + permute(A.x) )/ 256).rg;
}

float smoothDist(float x, float x0) {
	float t = x-x0;
	return t * t * t * (t * (t * 6 - 15) + 10);
	///return (3 - 2*t)*t*t;
}

float exp(float t) {
	return texture(expTableTexture, t/256).r;
}

float perlin() {
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
		value += Worley() * exp(i);
		fPosition *= Lacunarity;
	}
	remainder = Octaves - floor(Octaves);
	if(remainder > 0.0) {
		value += remainder * Worley() * exp(i);
	}
	return value;
}

float hmf() {
	float frequency;
	float result;
	float signal;
	float weight;
	float remainder;
	float i = 1;
	float offset = 0.4;
	result = (1 - abs(perlin()+ offset) ) * exp(0.0);
	weight = result;
	fPosition*=Lacunarity;

	for(i = 1.0; i < Octaves; i++) {
		if(weight > 1.0) {
			weight = 1;
		}
		signal = (1 - abs(perlin()+ offset) ) * exp(i);
		result += weight*signal;
		weight *= signal;
		fPosition *= Lacunarity;
	}
	remainder = Octaves - floor(Octaves);
	if ( remainder > 0.0 )
		result += remainder * (1 - abs(perlin()+ offset)  ) * exp(i);
	return( result );
}

void main() {
	fPosition = vPosition.xy / 2.0 + vec2(0.5,0.5);
	int x = int((fPosition.x * nCubes)/ 0.398150);
	int y = int((fPosition.y * nCubes)/ 0.398150);
	int seed = 702395077*x + 915488749*y;
	float f = poisson(seed >> 24)/5;
	f = Worley();
	fPosition = vPosition.xy / 2.0 + vec2(0.5,0.5);
	color = vec3(f);
}

