#version 330 core

out vec3 color;
in vec3 vPosition;
out vec2 fPosition;

uniform sampler1D permTableTexture;
uniform sampler1D gradTableTexture;
uniform sampler1D expTableTexture;
uniform sampler1D poissonTable;
uniform sampler1D fpTableX;
uniform sampler1D fpTableY;
uniform sampler2D pxTable;

uniform float Octaves;
uniform float Lacunarity;
uniform float H;
uniform float paramX;
uniform float paramY;
uniform float tileSize;

uniform int nCubes;

float permute(float x) {
	return texture(permTableTexture, x/256).r;
}

float poisson(float t) {
	return texture(poissonTable, t/256.0).r;
}

float getX(float t) {
	return texture(fpTableX, t/256.0).r;
}

float getY(float t) {
	return texture(fpTableY, t/256.0).r;
}

void addPoint(in vec2 cubeId, in vec2 at, inout float distVector[3], inout vec2 delta[3], inout float ID[3]) {

	vec2 d;
	vec2 f;
	float d2;
	float count;
	int i, j, index;
	float this_id;

	float seed = permute(cubeId.y + permute(cubeId.x));

	count = poisson(seed);
	for(j = 0; j <= count; j++) {
		this_id = seed;

		f.x= getX(permute(seed + j));

		f.y= getY(permute(seed + j));

		d.x = cubeId.x + f.x - at.x;
		d.y = cubeId.y + f.y - at.y;

		d2 = d.x * d.x + d.y * d.y;

		if(d2 < distVector[2]) {
			index = 3;

			while (index > 0 && d2 < distVector[index-1]) index--;

			for(i = 1; i >= index; i--) {
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
	float ID[3];

	vec2 at = fPosition.xy*nCubes;
	vec2 new_at = at;
	vec2 Fat;

	Fat.x = floor(new_at.x);
	Fat.y = floor(new_at.y);

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
			///return addPoint(Fat + vec2(0,0), new_at, distVector, delta, ID);
			addPoint(Fat + vec2(ii,jj), new_at, distVector, delta, ID);
		}
	}

	for(int i = 0; i < 3; i++) {
		distVector[i] = sqrt(distVector[i]);
	}

	return distVector[1] - distVector[0];

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
		value += perlin() * exp(i);
		fPosition *= Lacunarity;
	}
	remainder = Octaves - floor(Octaves);
	if(remainder > 0.0) {
		value += remainder * perlin() * exp(i);
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
	float offset = 0.0;
	result = (1 - abs(perlin()+offset)) * exp(0.0);
	weight = result;
	fPosition*=Lacunarity;

	for(i = 1.0; i < Octaves; i++) {
		if(weight > 1.0) {
			weight = 1;
		}
		signal = (1 - abs(perlin()+offset)) * exp(i);
		result += weight*signal;
		weight *= signal;
		fPosition *= Lacunarity;
	}
	remainder = Octaves - floor(Octaves);
	if ( remainder > 0.0 )
		result += remainder * (1 - abs(perlin()+offset) ) * exp(i);
	return( result );
}

void main() {
	fPosition = vPosition.xy / 2.0 + vec2(0.5,0.5);
	float f = Worley();
	color = vec3(f);
}

