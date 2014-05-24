// Copyright (C) 2014 - LGG EPFL

#include <iostream>
#include <sstream>
#include "common.h"

#include "vHeightMap2D.h"
#include "vHeightMap3D.h"
#include "fHeightMap2D.h"
#include "fHeightMap3D.h"
#include "vShader2D.h"
#include "vShader3D.h"
#include "fShader2D.h"
#include "fShader3D.h"
#include "vSkyBox.h"
#include "fSkyBox.h"
#include "vSkyBoxTex.h"
#include "fSkyBoxTex.h"
#include "vShadow2D.h"
#include "vShadow3D.h"
#include "fShadow2D.h"
#include "fShadow3D.h"

using namespace std;
using namespace opengp;


GLfloat Poisson_count[256] = {
	4, 3, 1, 1, 1, 2, 4, 2, 2, 2, 5, 1, 0, 2, 1, 2, 2, 0, 4, 3, 2, 1, 2, 1, 3, 2, 2, 4, 2, 2, 5, 1, 2, 3,
	2, 2, 2, 2, 2, 3, 2, 4, 2, 5, 3, 2, 2, 2, 5, 3, 3, 5, 2, 1, 3, 3, 4, 4, 2, 3, 0, 4, 2, 2, 2, 1, 3, 2,
	2, 2, 3, 3, 3, 1, 2, 0, 2, 1, 1, 2, 2, 2, 2, 5, 3, 2, 3, 2, 3, 2, 2, 1, 0, 2, 1, 1, 2, 1, 2, 2, 1, 3,
	4, 2, 2, 2, 5, 4, 2, 4, 2, 2, 5, 4, 3, 2, 2, 5, 4, 3, 3, 3, 5, 2, 2, 2, 2, 2, 3, 1, 1, 4, 2, 1, 3, 3,
	4, 3, 2, 4, 3, 3, 3, 4, 5, 1, 4, 2, 4, 3, 1, 2, 3, 5, 3, 2, 1, 3, 1, 3, 3, 3, 2, 3, 1, 5, 5, 4, 2, 2,
	4, 1, 3, 4, 1, 5, 3, 3, 5, 3, 4, 3, 2, 2, 1, 1, 1, 1, 1, 2, 4, 5, 4, 5, 4, 2, 1, 5, 1, 1, 2, 3, 3, 3,
	2, 5, 2, 3, 3, 2, 0, 2, 1, 1, 4, 2, 1, 3, 2, 1, 2, 2, 3, 2, 5, 5, 3, 4, 5, 5, 2, 4, 4, 5, 3, 2, 2, 2,
	1, 4, 2, 3, 3, 4, 2, 5, 4, 2, 4, 2, 2, 2, 4, 5, 3, 2
};


const GLsizei WIDTH = 1280;
const GLsizei HEIGHT = 1024;

const GLsizei TWIDTH = 256;
const GLsizei THEIGHT = 256;
const GLsizei TDEPTH = 256;

const GLenum mode = GL_FILL;

const GLint NCUBES = 4;
const GLuint MAX_OCTAVES = 512;
const GLfloat H = 1.0;
const GLfloat LACUNARITY = 2;
const GLfloat OCTAVES = log2(TWIDTH*THEIGHT) - 2;
const GLfloat PARAMX = 8;
const GLfloat PARAMY = 8;
const GLfloat PARAMZ = 8;


mat4 dmvp;
mat4 view;
mat4 model;
mat4 projection;

class SkyBox {
public:
	GLfloat s = 2;
	char* skyPath = "skybox.tga";
	GLuint skyBox;
	GLuint programID;
	GLuint vertexArray;
	GLuint skyTexture;
	vec3 cubeVertices[36];
	SkyBox() {
		vec3 cubeV[36] =
		{

			vec3(-s, -s, -s),
			vec3(-s, s, -s),
			vec3(s, -s, -s),
			vec3(-s, s, -s),
			vec3(s, -s, -s),
			vec3(s, s, -s),

			vec3(s, s, s),
			vec3(s, -s, s),
			vec3(s, s, -s),
			vec3(s, -s, s),
			vec3(s, s, -s),
			vec3(s, -s, -s),

			vec3(s, s, s),
			vec3(-s, s, s),
			vec3(s, -s, s),
			vec3(-s, s, s),
			vec3(s, -s, s),
			vec3(-s, -s, s),

			vec3(-s, -s, s),
			vec3(-s, -s, -s),
			vec3(s, -s, s),
			vec3(-s, -s, -s),
			vec3(s, -s, s),
			vec3(s, -s, -s),

			vec3(-s, s, -s),
			vec3(-s, -s, -s),
			vec3(-s, s, s),
			vec3(-s, -s, -s),
			vec3(-s, s, s),
			vec3(-s, -s, s),

			vec3(s, s, -s),
			vec3(-s, s, -s),
			vec3(s, s, s),

			vec3(-s, s, -s),
			vec3(s, s, s),
			vec3(-s, s, s)
		};
		for (int i = 0; i <= 35; i++) {
			cubeVertices[i] = cubeV[i];
		}
		init();
	}

	void init() {

		skyTexture = genSkyBox();

		programID = compile_shaders(vSkyBox, fSkyBox);
		glUseProgram(programID);

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glViewport(0.0, 0.0, WIDTH, HEIGHT);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
		GLuint skyLoc = glGetUniformLocation(programID, "skyTexture");
		glUniform1i(skyLoc, 2);

		GLuint cube;
		glGenBuffers(1, &cube);
		glBindBuffer(GL_ARRAY_BUFFER, cube);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLuint position = glGetAttribLocation(programID, "position");
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(vec3), &cubeVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(position, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
		glEnableVertexAttribArray(position);
	}

	GLuint genSkyBox() {
		programID = compile_shaders(vSkyBoxTex, fSkyBoxTex);
		skyBox = loadImage(skyPath);
		glUseProgram(programID);
		GLuint skyt;
		glGenTextures(1, &skyt);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyt);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, skyBox);
		GLuint skyLoc = glGetUniformLocation(programID, "skyTex");
		glUniform1i(skyLoc, 1);

		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		GLuint framebufferVAO;
		glGenVertexArrays(1, &framebufferVAO);
		glBindVertexArray(framebufferVAO);
		
		glViewport(0.0, 0.0, TWIDTH, THEIGHT);

		float s = 1.1;
		vec3 vertices[4] = {
			vec3(-s, -s, 0.0),
			vec3(-s, s, 0.0),
			vec3(s, -s, 0.0),
			vec3(s, s, 0.0)
		};

		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, 4*sizeof(vec3), &vertices[0], GL_STATIC_DRAW);

		GLuint position = glGetAttribLocation(programID, "position");

		glVertexAttribPointer(position, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);

		glEnableVertexAttribArray(position);

		

		GLuint layer = 0;
		GLuint layerLoc = glGetUniformLocation(programID, "layer");
		for (layer = 0; layer < 6; layer++){
			glUniform1i(layerLoc, layer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, skyt, 0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		glViewport(0.0, 0.0, WIDTH, HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return skyt;
	}

	GLuint loadImage(const char *imagepath){
		GLuint textureID;
		glGenTextures(ONE, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glfwLoadTexture2D(imagepath, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;

	}

	void draw() {
		glUseProgram(programID);
		glBindVertexArray(vertexArray);
		GLuint mvpid = glGetUniformLocation(programID, "MVPID");
		mat4 MVP = projection*view*model;
		glUniformMatrix4fv(mvpid, 1, GL_FALSE, MVP.data());
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Terrain2D {
public:

	GLuint programID;
	GLuint terrainID;
	GLuint vertexArray;
	GLuint noiseTexture;
	vec3 light_direction = vec3(0.0, 0.0, -1.0);

	char* snowPath = "snow.tga";
	char* grassPath = "grass.tga";
	char* rockPath = "rock.tga";
	char* sandPath = "sand.tga";

	GLfloat permTable[256];
	vec3 gradTable[256];
	GLfloat fpX[256];
	GLfloat fpY[256];
	GLfloat expArray[MAX_OCTAVES];

	GLfloat tileSize = 1;
	GLuint squareNum = 512;

	GLuint framebufferVAO;

	const GLsizei TWIDTH = 1024;
	const GLsizei THEIGHT = 1024;

	vector<vec3> terrain;

	void init(int seed) {

		programID = compile_shaders(vShader2D, fShader2D);

		glUseProgram(programID);

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		srand(seed);

		for (int i = 0; i < 255; i++) {
			permTable[i] = i;
		}
		for (int i = 0; i < 255; i++) {
			float r1 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
			float r2 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
			float r3 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
			int t = rand() % 256;
			int tmp = permTable[i];
			permTable[i] = permTable[t];
			permTable[t] = tmp;
			float n = sqrt(r1*r1 + r2*r2 + r3*r3);
			gradTable[i] = vec3(r1 / n, r2 / n, r3 / n);
		}
		for (int i = 0; i < 255; i++) {
			fpX[i] = rand() / ((float)RAND_MAX);
			fpY[i] = rand() / ((float)RAND_MAX);
		}

		genMap();
		genNoiseTexture();
		loadTextures();
	}

	Terrain2D(int seed) {
		init(seed);
	}

	void use() {
		glUseProgram(programID);
	}

	void genMap() {
		float increment = tileSize / squareNum;
		float off = tileSize / 2.0;
		float current_incr_i = 0.0;
		float current_incr_j = 0.0;

		for (int i = 0; i < squareNum; i++) {

			vec3 low_left = vec3(current_incr_i - off, -off, 0.0);
			vec3 low_right = low_left + vec3(increment, 0.0, 0.0);
			vec3 up_left = low_left + vec3(0.0, increment, 0.0);
			vec3 up_right = low_right + vec3(0.0, increment, 0.0);

			terrain.push_back(low_left);
			terrain.push_back(up_left);
			terrain.push_back(low_right);
			terrain.push_back(low_right);
			terrain.push_back(up_left);
			terrain.push_back(up_right);

			current_incr_j = 0.0;
			for (int j = 1; j < squareNum; j++) {

				vec3 low_left = vec3(current_incr_i - off, current_incr_j - off, 0.0);
				vec3 low_right = low_left + vec3(increment, 0.0, 0.0);
				vec3 up_left = low_left + vec3(0.0, increment, 0.0);
				vec3 up_right = low_right + vec3(0.0, increment, 0.0);

				terrain.push_back(low_left);
				terrain.push_back(up_left);
				terrain.push_back(low_right);
				terrain.push_back(low_right);
				terrain.push_back(up_left);
				terrain.push_back(up_right);

				current_incr_j += increment;
			}
			current_incr_i += increment;
		}

		glPolygonMode(GL_FRONT_AND_BACK, mode);

		glGenBuffers(1, &terrainID);
		glBindBuffer(GL_ARRAY_BUFFER, terrainID);
		glEnableVertexAttribArray(0);
		glBufferData(GL_ARRAY_BUFFER, terrain.size()*sizeof(vec3), &(terrain[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
		GLuint lightLoc = glGetUniformLocation(programID, "light_pos");
		glUniform3f(lightLoc, -light_direction.x(), -light_direction.y(), -light_direction.z());
	}

	void genNoiseTexture() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		GLuint heightMapID = compile_shaders(vHeightMap2D, fHeightMap2D);
		glUseProgram(heightMapID);
		GLuint frameBufferObject;
		glGenFramebuffers(1, &frameBufferObject);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

		GLfloat frequency = 1.0;
		for (int i = 0; i < MAX_OCTAVES; i++) {
			expArray[i] = pow(frequency, -H);
			frequency *= LACUNARITY;
		}

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TWIDTH, THEIGHT, 0, GL_RED, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE1);
		GLuint permTableTexture;
		glGenTextures(1, &permTableTexture);
		glBindTexture(GL_TEXTURE_1D, permTableTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &permTable[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint permTableLoc = glGetUniformLocation(heightMapID, "permTableTexture");
		glUniform1i(permTableLoc, 1);

		glActiveTexture(GL_TEXTURE2);
		GLuint gradTableTexture;
		glGenTextures(1, &gradTableTexture);
		glBindTexture(GL_TEXTURE_1D, gradTableTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, 256, 0, GL_RGB, GL_FLOAT, &gradTable[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint gradTableLoc = glGetUniformLocation(heightMapID, "gradTableTexture");
		glUniform1i(gradTableLoc, 2);

		glActiveTexture(GL_TEXTURE3);
		GLuint expArrayTexture;
		glGenTextures(1, &expArrayTexture);
		glBindTexture(GL_TEXTURE_1D, expArrayTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &expArray[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint expArrayLoc = glGetUniformLocation(heightMapID, "expTableTexture");
		glUniform1i(expArrayLoc, 3);

		glActiveTexture(GL_TEXTURE4);
		GLuint poissonTableTexture;
		glGenTextures(1, &poissonTableTexture);
		glBindTexture(GL_TEXTURE_1D, poissonTableTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &Poisson_count[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint poissonTableLoc = glGetUniformLocation(heightMapID, "poissonTable");
		glUniform1i(poissonTableLoc, 4);

		glActiveTexture(GL_TEXTURE5);
		GLuint featurePointXTexture;
		glGenTextures(1, &featurePointXTexture);
		glBindTexture(GL_TEXTURE_1D, featurePointXTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &fpX[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint fpXTableLoc = glGetUniformLocation(heightMapID, "fpTableX");
		glUniform1i(fpXTableLoc, 5);

		glActiveTexture(GL_TEXTURE6);
		GLuint featurePointYTexture;
		glGenTextures(1, &featurePointYTexture);
		glBindTexture(GL_TEXTURE_1D, featurePointYTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &fpY[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint fpYTableLoc = glGetUniformLocation(heightMapID, "fpTableY");
		glUniform1i(fpYTableLoc, 6);

		GLuint Hloc = glGetUniformLocation(heightMapID, "H");
		glUniform1f(Hloc, H);

		GLuint Lacloc = glGetUniformLocation(heightMapID, "Lacunarity");
		glUniform1f(Lacloc, LACUNARITY);

		GLuint Ocloc = glGetUniformLocation(heightMapID, "Octaves");
		glUniform1f(Ocloc, OCTAVES);

		GLuint Paxloc = glGetUniformLocation(heightMapID, "paramX");
		GLuint Payloc = glGetUniformLocation(heightMapID, "paramY");
		glUniform1f(Paxloc, PARAMX);
		glUniform1f(Payloc, PARAMY);

		GLuint Cubloc = glGetUniformLocation(heightMapID, "nCubes");
		glUniform1i(Cubloc, NCUBES);


		glGenVertexArrays(1, &framebufferVAO);
		glBindVertexArray(framebufferVAO);

		glViewport(0.0, 0.0, TWIDTH, THEIGHT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, noiseTexture, 0);

		vec3 quad[4] = {
			vec3(-1.0, -1.0, 0.0),
			vec3(-1.0, 1.0, 0.0),
			vec3(1.0, -1.0, 0.0),
			vec3(1.0, 1.0, 0.0)
		};

		GLuint quadID;
		glGenBuffers(1, &quadID);
		glBindBuffer(GL_ARRAY_BUFFER, quadID);

		GLuint position = glGetAttribLocation(heightMapID, "position");
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), &quad[0], GL_STATIC_DRAW);
		glVertexAttribPointer(position, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
		glEnableVertexAttribArray(position);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(programID);
	}

	GLuint loadImage(const char *imagepath){
		GLuint textureID;
		glGenTextures(ONE, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glfwLoadTexture2D(imagepath, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}

	void loadTextures() {
		GLuint grassId = loadImage(grassPath);
		GLuint rockId = loadImage(rockPath);
		GLuint sandId = loadImage(sandPath);
		GLuint snowId = loadImage(snowPath);
		GLuint depthMap = genShadowMap();

		glUseProgram(programID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		GLuint noiseloc = glGetUniformLocation(programID, "noiseTexture");
		glUniform1i(noiseloc, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, sandId);
		glUniform1i(glGetUniformLocation(programID, "sandTexture"), 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, grassId);
		glUniform1i(glGetUniformLocation(programID, "grassTexture"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rockId);
		glUniform1i(glGetUniformLocation(programID, "rockTexture"), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, snowId);
		glUniform1i(glGetUniformLocation(programID, "snowTexture"), 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(glGetUniformLocation(programID, "shadowMap"), 5);
	}

	GLuint genShadowMap() {
		glViewport(0.0, 0.0, WIDTH, HEIGHT);

		GLuint program = compile_shaders(vShadow2D, fShadow2D);
		glUseProgram(program);

		GLuint depthMap;

		mat4 proj = Eigen::perspective(45.0f, 4.0f / 3.0f, 0.1f, 25.0f);

		vec3 pos = -light_direction;
		vec3 look(0.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		mat4 dview = Eigen::lookAt(pos, look, up);

		GLuint fb;
		glGenFramebuffers(1, &fb);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glViewport(0.0, 0.0, 1024, 1024);

		dmvp = proj*dview;

		glGenTextures(1, &depthMap);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glUniform1i(glGetUniformLocation(program, "noiseTexture"), 1);

		glUniformMatrix4fv(glGetUniformLocation(program, "dMVP"), 1, DONT_TRANSPOSE, dmvp.data());

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDrawBuffer(GL_NONE);
		glBindVertexArray(vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, terrain.size());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0.0, 0.0, WIDTH, HEIGHT);
		return depthMap;
	}

	void draw() {
		glBindVertexArray(vertexArray);
		glUseProgram(programID);
		mat4 MV = view*model;
		glUniformMatrix4fv(glGetUniformLocation(programID, "model_view"), 1, GL_FALSE, MV.data());
		glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, projection.data());
		glUniformMatrix4fv(glGetUniformLocation(programID, "dMVP"), 1, DONT_TRANSPOSE, dmvp.data());
		glDrawArrays(GL_TRIANGLES, 0, terrain.size());
	}

};

class Terrain3D {
public:
	vec3 light_position = vec3(8.0, 0.0, 0.0);
	char* snowPath = "snow.tga";
	char* grassPath = "grass.tga";
	char* rockPath = "rock.tga";
	char* sandPath = "sand.tga";
	GLuint vertexArray;
	GLfloat permTable[256];
	vec3 gradTable[256];
	GLuint terrainID;
	GLfloat expArray[256];
	GLfloat fpX[256];
	GLfloat fpY[256];
	GLfloat fpZ[256];
	vector<vec3> terrain3D;
	GLuint programID;
	GLuint noiseTexture;
	GLuint framebufferVAO;
	const float squareNum = 512;
	const int recursionLevel = 7;
	//this correction factor aims to reduce the visibility of some artifacts, until I figure out what's causing them (perlin noise artifact when x,y, or z = -1 or 1)
	//It should be set to a value bigger than 1.0.
	const float correctionFactor = 1.1;

	//This can be offset with little damage by adding a scaling factor of 1/correctionFactor to the MV matrix.
	//However, it's completely useless here.

	void init(int seed) {

		programID = compile_shaders(vShader3D, fShader3D);
		glUseProgram(programID);

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);


		genMap3D();
		srand(seed);

		for (int i = 0; i < 255; i++) {
			permTable[i] = i;
		}
		for (int i = 0; i < 255; i++) {
			float r1 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
			float r2 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
			float r3 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
			int t = rand() % 256;
			int tmp = permTable[i];
			permTable[i] = permTable[t];
			permTable[t] = tmp;
			float n = sqrt(r1*r1 + r2*r2 + r3*r3);
			gradTable[i] = vec3(r1 / n, r2 / n, r3 / n);
		}
		for (int i = 0; i < 255; i++) {
			fpX[i] = rand() / ((float)RAND_MAX);
			fpY[i] = rand() / ((float)RAND_MAX);
			fpZ[i] = rand() / ((float)RAND_MAX);
		}

		textureGeneration();
		loadTextures();
	}

	Terrain3D(int seed) {
		init(seed);
	}

	float length(vec3 a) {
		return sqrt(a.x() * a.x() + a.y()*a.y() + a.z()*a.z());
	}

	void indsToTriangle(vec3 triangles[12], int i, int j, int k) {
		//we double the edge vertices so OpenGL treats ever triangle as an individual triangle.
		terrain3D.push_back(triangles[i] / (correctionFactor*length(triangles[i])));
		terrain3D.push_back(triangles[j] / (correctionFactor * length(triangles[j])));
		terrain3D.push_back(triangles[k] / (correctionFactor * length(triangles[k])));
	}

	void genMap3D() {
		//Initial icosahedron
		vec3 triangles[12];
		float t = (1.0 + sqrt(5.0)) / 2.0;

		triangles[0] = (vec3(-1, t, 0));
		triangles[1] = (vec3(1, t, 0));
		triangles[2] = (vec3(-1, -t, 0));
		triangles[3] = (vec3(1, -t, 0));

		triangles[4] = (vec3(0, -1, t));
		triangles[5] = (vec3(0, 1, t));
		triangles[6] = (vec3(0, -1, -t));
		triangles[7] = (vec3(0, 1, -t));

		triangles[8] = (vec3(t, 0, -1));
		triangles[9] = (vec3(t, 0, 1));
		triangles[10] = (vec3(-t, 0, -1));
		triangles[11] = (vec3(-t, 0, 1));

		//Initial 20 Triangles icosahedron
		indsToTriangle(triangles, 0, 11, 5);
		indsToTriangle(triangles, 0, 5, 1);
		indsToTriangle(triangles, 0, 1, 7);
		indsToTriangle(triangles, 0, 7, 10);
		indsToTriangle(triangles, 0, 10, 11);

		indsToTriangle(triangles, 1, 5, 9);
		indsToTriangle(triangles, 5, 11, 4);
		indsToTriangle(triangles, 11, 10, 2);
		indsToTriangle(triangles, 10, 7, 6);
		indsToTriangle(triangles, 7, 1, 8);

		indsToTriangle(triangles, 3, 9, 4);
		indsToTriangle(triangles, 3, 4, 2);
		indsToTriangle(triangles, 3, 2, 6);
		indsToTriangle(triangles, 3, 6, 8);
		indsToTriangle(triangles, 3, 8, 9);

		indsToTriangle(triangles, 4, 9, 5);
		indsToTriangle(triangles, 2, 4, 11);
		indsToTriangle(triangles, 6, 2, 10);
		indsToTriangle(triangles, 8, 6, 7);
		indsToTriangle(triangles, 9, 8, 1);


		//testing it out
		for (int i = 0; i < recursionLevel; i++) {
			vector<vec3> terrain2(0);
			int n = terrain3D.size();
			for (int j = 0; j < n / 3; j++) {
				//we cut every triangle into 4 triangles.
				vec3 A = terrain3D[3 * j];
				vec3 B = terrain3D[3 * j + 1];
				vec3 C = terrain3D[3 * j + 2];

				vec3 midAB = (A + B) / 2.0;
				midAB = midAB / (correctionFactor*length(midAB));
				vec3 midAC = (A + C) / 2.0;
				midAC = midAC / (correctionFactor*length(midAC));
				vec3 midBC = (B + C) / 2.0;
				midBC = midBC / (correctionFactor*length(midBC));

				terrain2.push_back(A);
				terrain2.push_back(midAB);
				terrain2.push_back(midAC);

				terrain2.push_back(midAB);
				terrain2.push_back(B);
				terrain2.push_back(midBC);

				terrain2.push_back(midAC);
				terrain2.push_back(midBC);
				terrain2.push_back(C);

				terrain2.push_back(midAB);
				terrain2.push_back(midBC);
				terrain2.push_back(midAC);
			}
			terrain3D = terrain2;
		}

		glPolygonMode(GL_FRONT_AND_BACK, mode);

		glGenBuffers(1, &terrainID);
		glBindBuffer(GL_ARRAY_BUFFER, terrainID);
		glEnableVertexAttribArray(0);
		GLuint lightLoc = glGetUniformLocation(programID, "light_pos");
		glUniform3f(lightLoc, light_position.x(), light_position.y(), light_position.z());
		glBufferData(GL_ARRAY_BUFFER, terrain3D.size()*sizeof(vec3), &(terrain3D[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
	}

	void textureGeneration() {

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		GLuint heightMapID = compile_shaders(vHeightMap3D, fHeightMap3D);
		glUseProgram(heightMapID);

		GLuint frameBufferObject;
		glGenFramebuffers(1, &frameBufferObject);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

		GLfloat frequency = 1.0;
		for (int i = 0; i < MAX_OCTAVES; i++) {
			expArray[i] = pow(frequency, -H);
			frequency *= LACUNARITY;
		}

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_3D, noiseTexture);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, TWIDTH, THEIGHT, TWIDTH, 0, GL_RED, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glActiveTexture(GL_TEXTURE1);
		GLuint permTableTexture;
		glGenTextures(1, &permTableTexture);
		glBindTexture(GL_TEXTURE_1D, permTableTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &permTable[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint permTableLoc = glGetUniformLocation(heightMapID, "permTableTexture");
		glUniform1i(permTableLoc, 1);

		glActiveTexture(GL_TEXTURE2);
		GLuint gradTableTexture;
		glGenTextures(1, &gradTableTexture);
		glBindTexture(GL_TEXTURE_1D, gradTableTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, 256, 0, GL_RGB, GL_FLOAT, &gradTable[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint gradTableLoc = glGetUniformLocation(heightMapID, "gradTableTexture");
		glUniform1i(gradTableLoc, 2);

		glActiveTexture(GL_TEXTURE3);
		GLuint expArrayTexture;
		glGenTextures(1, &expArrayTexture);
		glBindTexture(GL_TEXTURE_1D, expArrayTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &expArray[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint expArrayLoc = glGetUniformLocation(heightMapID, "expTableTexture");
		glUniform1i(expArrayLoc, 3);

		glActiveTexture(GL_TEXTURE4);
		GLuint poissonTableTexture;
		glGenTextures(1, &poissonTableTexture);
		glBindTexture(GL_TEXTURE_1D, poissonTableTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &Poisson_count[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint poissonTableLoc = glGetUniformLocation(heightMapID, "poissonTable");
		glUniform1i(poissonTableLoc, 4);

		glActiveTexture(GL_TEXTURE5);
		GLuint featurePointXTexture;
		glGenTextures(1, &featurePointXTexture);
		glBindTexture(GL_TEXTURE_1D, featurePointXTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &fpX[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint fpXTableLoc = glGetUniformLocation(heightMapID, "fpTableX");
		glUniform1i(fpXTableLoc, 5);

		glActiveTexture(GL_TEXTURE6);
		GLuint featurePointYTexture;
		glGenTextures(1, &featurePointYTexture);
		glBindTexture(GL_TEXTURE_1D, featurePointYTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &fpY[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint fpYTableLoc = glGetUniformLocation(heightMapID, "fpTableY");
		glUniform1i(fpYTableLoc, 6);

		glActiveTexture(GL_TEXTURE7);
		GLuint featurePointZTexture;
		glGenTextures(1, &featurePointZTexture);
		glBindTexture(GL_TEXTURE_1D, featurePointZTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, &fpZ[0]);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLuint fpZTableLoc = glGetUniformLocation(heightMapID, "fpTableZ");
		glUniform1i(fpZTableLoc, 7);

		GLuint Hloc = glGetUniformLocation(heightMapID, "H");
		glUniform1f(Hloc, H);
		GLuint Lacloc = glGetUniformLocation(heightMapID, "Lacunarity");
		glUniform1f(Lacloc, LACUNARITY);
		GLuint Ocloc = glGetUniformLocation(heightMapID, "Octaves");
		glUniform1f(Ocloc, OCTAVES);
		GLuint Paxloc = glGetUniformLocation(heightMapID, "paramX");
		GLuint Payloc = glGetUniformLocation(heightMapID, "paramY");
		glUniform1f(Paxloc, PARAMX);
		glUniform1f(Payloc, PARAMY);
		GLuint Pazloc = glGetUniformLocation(heightMapID, "paramZ");
		glUniform1f(Pazloc, PARAMZ);
		GLuint Cubloc = glGetUniformLocation(heightMapID, "nCubes");
		glUniform1i(Cubloc, NCUBES);

		glGenVertexArrays(1, &framebufferVAO);
		glBindVertexArray(framebufferVAO);

		glViewport(0.0, 0.0, TWIDTH, THEIGHT);

		vec3 quad[4] = {
			vec3(-1.0, -1.0, 0.0),
			vec3(-1.0, 1.0, 0.0),
			vec3(1.0, -1.0, 0.0),
			vec3(1.0, 1.0, 0.0),
		};

		for (int i = 1; i <= TDEPTH; i++) {
			glBindTexture(GL_TEXTURE_3D, noiseTexture);
			glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, noiseTexture, 0, i);
			GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, DrawBuffers);

			for (int j = 0; j < 4; j++) {
				float z = i / static_cast<float>(TDEPTH + 1) * 2 - 1.0;
				quad[j] = vec3(quad[j].x(), quad[j].y(), z);
			}

			GLuint quadID;
			glGenBuffers(1, &quadID);
			glBindBuffer(GL_ARRAY_BUFFER, quadID);
			GLuint position = glGetAttribLocation(heightMapID, "position");
			glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), &quad[0], GL_STATIC_DRAW);
			glVertexAttribPointer(position, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
			glEnableVertexAttribArray(position);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(programID);
		//glBindVertexArray(vertexArray);
		glViewport(0.0, 0.0, WIDTH, HEIGHT);
	}

	void use() {
		glUseProgram(programID);
	}

	GLuint loadImage(const char *imagepath){
		GLuint textureID;
		glGenTextures(ONE, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glfwLoadTexture2D(imagepath, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}

	void loadTextures() {
		GLuint grassId = loadImage(grassPath);
		GLuint rockId = loadImage(rockPath);
		GLuint sandId = loadImage(sandPath);
		GLuint snowId = loadImage(snowPath);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, noiseTexture);
		GLuint noiseloc = glGetUniformLocation(programID, "noiseTexture");
		glUniform1i(noiseloc, 0);


		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, sandId);
		glUniform1i(glGetUniformLocation(programID, "sandTexture"), 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, grassId);
		glUniform1i(glGetUniformLocation(programID, "grassTexture"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rockId);
		glUniform1i(glGetUniformLocation(programID, "rockTexture"), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, snowId);
		glUniform1i(glGetUniformLocation(programID, "snowTexture"), 4);

	}

	void draw() {
		glBindVertexArray(vertexArray);
		mat4 MV = view*model;
		glUniformMatrix4fv(glGetUniformLocation(programID, "model_view"), 1, GL_FALSE, MV.data());
		glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, projection.data());
		glDrawArrays(GL_TRIANGLES, 0, terrain3D.size());
	}

};

void update_matrix_stack(const mat4 &_model) {
	model = _model;

	projection = Eigen::perspective(45.0f, 4.0f / 3.0f, 0.1f, 25.0f);

	vec3 cam_pos(0.0f, 0.0f, 5.0f);
	vec3 cam_look(0.0f, 0.0f, 0.0f);
	vec3 cam_up(0.0f, 1.0f, 0.0f);
	view = Eigen::lookAt(cam_pos, cam_look, cam_up);
}

#define Terrain Terrain2D

Terrain *p;
SkyBox *s;

void display() {
	glPointSize(2.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	p->use();
	p->draw();
	//s->draw();
}

void init() {

	s = new SkyBox();
	p = new Terrain(8545);
	update_matrix_stack(mat4::Identity());

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glPointSize(5.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

int main(int, char**) {
	cout << GL_MAX_3D_TEXTURE_SIZE;
	glfwInitWindowSize(WIDTH, HEIGHT);
	glfwCreateWindow("Project 1");
	glfwDisplayFunc(display);
	init();
	glfwTrackball(update_matrix_stack);
	glfwMainLoop();
	return EXIT_SUCCESS;
}

