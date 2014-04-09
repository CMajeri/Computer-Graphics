// Copyright (C) 2014 - LGG EPFL

#include <iostream>
#include <sstream>
#include "common.h"

#include "heightMapGen_vshader.h"
#include "heightMapGen_gshader.h"
#include "heightMapGen_fshader.h"
#include "vshader2.h"
#include "fshader2.h"
#include "gshader2.h"

using namespace std;
using namespace opengp;

GLfloat permTable[256] = {};

vec3 gradTable[256] = {};

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

GLfloat fpX[256];
GLfloat fpY[256];
GLfloat fpZ[256];

const GLsizei WIDTH = 1280;
const GLsizei HEIGHT = 1024;
const GLfloat tileSize = 1;
const GLsizei TWIDTH = 256;
const GLsizei THEIGHT = 256;
const GLsizei TDEPTH = 256;
vector<vec3> terrain(0);
vector<vec3> terrain3D(0);
const float squareNum = 512;
const int recursionLevel = 7;
int vNum = 6 * squareNum * squareNum;


GLint NCUBES = 40;
const GLuint MAX_OCTAVES = 512;
GLfloat H = 1.0;
GLfloat LACUNARITY = 20;
GLfloat OCTAVES = log2(TWIDTH*THEIGHT) - 2;
GLfloat PARAMX = 5;
GLfloat PARAMY = 5;
GLfloat PARAMZ = 5;
GLfloat expArray[MAX_OCTAVES];
GLint seed = 4941255;

vec3 light_position = vec3(5.0, 0.0, 5.0);


mat4 view;
mat4 model;
mat4 projection;


GLuint vertexArray;
GLuint terrainID;
GLuint programID;
GLuint framebufferVAO;
GLuint heightMapID;
GLuint noiseTexture;



void update_matrix_stack(const mat4 &_model) {
	model = _model;

	projection = Eigen::perspective(45.0f, 4.0f / 3.0f, 0.1f, 15.0f);

	vec3 cam_pos(0.0f, 0.0f, 3.0);
	vec3 cam_look(0.0f, 0.0f, 0.0f);
	vec3 cam_up(0.0f, 1.0f, 0.0f);
	view = Eigen::lookAt(cam_pos, cam_look, cam_up);
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

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glGenBuffers(1, &terrainID);
	glBindBuffer(GL_ARRAY_BUFFER, terrainID);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, vNum*sizeof(vec3), &(terrain[0]), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, noiseTexture);
}

float length(vec3 a) {
	return sqrt(a.x() * a.x() + a.y()*a.y() + a.z()*a.z());
}

void indsToTriangle(vec3 triangles[12], int i, int j, int k) {
	//we double the edge vertices so OpenGL treats ever triangle as an individual triangle.
	terrain3D.push_back(triangles[i] /length(triangles[i]));
	terrain3D.push_back(triangles[j] / length(triangles[j]));
	terrain3D.push_back(triangles[k] / length(triangles[k]));
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
			midAB = midAB / length(midAB);
			vec3 midAC = (A + C) / 2.0;
			midAC = midAC / length(midAC);
			vec3 midBC = (B + C) / 2.0;
			midBC = midBC / length(midBC);

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

	vNum = terrain3D.size();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glGenBuffers(1, &terrainID);
	glBindBuffer(GL_ARRAY_BUFFER, terrainID);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, terrain3D.size()*sizeof(vec3), &(terrain3D[0]), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, noiseTexture);

}

void textureGeneration() {



	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	heightMapID = compile_shaders(heightMapGen_vshader, heightMapGen_fshader);
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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, TWIDTH, THEIGHT, TDEPTH, 0, GL_RED, GL_FLOAT, 0);
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


	update_matrix_stack(mat4::Identity());

	glGenVertexArrays(1, &framebufferVAO);
	glBindVertexArray(framebufferVAO);

	glViewport(0.0, 0.0, TWIDTH, THEIGHT);

	vec3 quad[4] = {
		vec3(-1.0, -1.0, 0.0),
		vec3(-1.0, 1.0, 0.0),
		vec3(1.0, -1.0, 0.0),
		vec3(1.0, 1.0, 0.0),
	};

	for (int i = 0; i < TDEPTH; i++) {
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
	glBindVertexArray(vertexArray);
	glViewport(0.0, 0.0, WIDTH, HEIGHT);
}

void display() {
	glPointSize(2.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLuint PID = glGetUniformLocation(programID, "projection");
	GLuint MVID = glGetUniformLocation(programID, "model_view");
	mat4 MV = view * model;
	glUniformMatrix4fv(MVID, 1, GL_FALSE, MV.data());
	glUniformMatrix4fv(PID, 1, GL_FALSE, projection.data());
	glDrawArrays(GL_TRIANGLES, 0, vNum);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void init() {

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	update_matrix_stack(mat4::Identity());

	programID = compile_shaders(vshader2, fshader2);
	glUseProgram(programID);
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
		gradTable[i] = vec3(r1/n, r2/n, r3/n);
	}
	for (int i = 0; i < 255; i++) {
		fpX[i] = rand() / ((float)RAND_MAX);
		fpY[i] = rand() / ((float)RAND_MAX);
		fpZ[i] = rand() / ((float)RAND_MAX);
	}


	GLuint lightLoc = glGetUniformLocation(programID, "light_pos");
	glUniform3f(lightLoc, light_position.x(), light_position.y(), light_position.z());
	GLuint Tsloc = glGetUniformLocation(programID, "tileSize");
	glUniform1f(Tsloc, tileSize);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glPointSize(5.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

int main(int, char**) {
	glfwInitWindowSize(WIDTH, HEIGHT);
	glfwCreateWindow("Project 1");
	glfwDisplayFunc(display);
	init();
	textureGeneration();
	genMap3D();
	glfwTrackball(update_matrix_stack);
	glfwMainLoop();
	return EXIT_SUCCESS;
}

