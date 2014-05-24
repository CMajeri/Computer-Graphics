#version 330 core

out vec3 color;
in vec3 vPosition;
out vec3 fPosition;


uniform sampler1D permTableTexture;
uniform sampler1D gradTableTexture;
uniform sampler1D expTableTexture;
uniform sampler1D poissonTable;
uniform sampler1D fpTableX;
uniform sampler1D fpTableY;
uniform sampler1D fpTableZ;

uniform float Octaves;
uniform float Lacunarity;
uniform float H;
uniform float paramX;
uniform float paramY;
uniform float paramZ;
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

float getZ(float t) {
	return texture(fpTableZ, t/256.0).r;
}

void addPoint(in vec3 cubeId, in vec3 at, inout float distVector[3]) {

	vec3 d;
	vec3 f;
	float d2;
	float count;
	int i, j, index;

	float seed = permute(cubeId.z + permute(cubeId.y + permute(cubeId.x)));

	count = poisson(seed);
	for(j = 0; j <= count; j++) {
		f.x= getX(permute(seed + j));

		f.y= getY(permute(seed + j));

		f.z= getZ(permute(seed + j));

		d.x = cubeId.x + f.x - at.x;
		d.y = cubeId.y + f.y - at.y;
		d.z = cubeId.z + f.z - at.z;

		d2 = d.x * d.x + d.y * d.y + d.z*d.z;

		if(d2 < distVector[2]) {
			index = 3;
			while (index > 0 && d2 < distVector[index-1]) index--;
			for(i = 1; i >= index; i--) {
				distVector[i+1]=distVector[i];
			}
			distVector[index]=d2;
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


	vec3 at = fPosition*nCubes;
	vec3 new_at = at;
	vec3 Fat;

	Fat.x = floor(new_at.x);
	Fat.y = floor(new_at.y);
	Fat.z = floor(new_at.z);

	int ii, jj, id = 0, ie = 0, jd = 0, je = 0, ke = 0, kd = 0, kk;

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

	if(Fat.z > 0) {
		kd = -1;
	}

	if(Fat.z < nCubes - 1) {
		ke = 1;
	}

	for (ii = id; ii<=ie; ii++) {
		for (jj = jd; jj<=je; jj++) {
			for(kk = kd; kk<=ke; kk++) {
				addPoint(Fat + vec3(ii, jj, kk), new_at, distVector);
			}
		}
	}

	for(int i = 0; i < 3; i++) {
		distVector[i] = sqrt(distVector[i]);
	}

	return distVector[1] - distVector[0];

}



vec3 gradient(vec3 A) {
	return texture(gradTableTexture, ( permute (A.z + permute( A.y + permute(A.x) ) ) )/ 256).rgb;
}

float fade(float x) {
	float t = x-floor(x);
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float t){
	return a+t*(b-a);
}

float exp(float t) {
	return texture(expTableTexture, t/256).r;
}

float perlin() {

	vec3 P = fPosition * vec3(paramX, paramY, paramZ);

	vec3 Fp = floor(P);

	vec3 A = vec3(Fp.x, Fp.y, Fp.z);
	float a = dot(gradient(A), P-A);

	vec3 B = vec3(Fp.x, Fp.y+1, Fp.z);
	float b = dot(gradient(B), P-B);
	
	vec3 C = vec3(Fp.x+1, Fp.y+1, Fp.z);
	float c = dot(gradient(C), P-C);

	vec3 D = vec3(Fp.x+1, Fp.y, Fp.z);
	float d = dot(gradient(D), P-D);

	vec3 Au = vec3(Fp.x, Fp.y, Fp.z+1);
	float au = dot(gradient(Au), P-Au);

	vec3 Bu = vec3(Fp.x, Fp.y+1, Fp.z+1);
	float bu = dot(gradient(Bu), P-Bu);
	
	vec3 Cu = vec3(Fp.x+1, Fp.y+1, Fp.z+1);
	float cu = dot(gradient(Cu), P-Cu);

	vec3 Du = vec3(Fp.x+1, Fp.y, Fp.z+1);
	float du = dot(gradient(Du), P-Du);
	
	float Sx = fade(P.x);
	float Sy = fade(P.y);
	float Sz = fade(P.z);
	
	float AD = lerp(a, d, Sx);
	
	float AuDu = lerp(au, du, Sx);
	
	float BC = lerp(b, c, Sx);
	
	float BuCu = lerp(bu, cu, Sx);

	float Ydown = lerp(AD, BC, Sy);
	float Yup = lerp(AuDu, BuCu, Sy);

	float totalContrib = lerp(Ydown, Yup, Sz);

	return totalContrib;
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
	fPosition = vPosition / 2.0 + 0.5;
	highp float f = fBm();
	color = vec3(f);
}

