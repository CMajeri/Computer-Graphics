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

GLubyte permTable[256] = {};

//Trust me, there are 256 of them.
vec3 gradTable[256] = {};


GLsizei WIDTH = 1280;
GLsizei HEIGHT = 1024;

mat4 view;
mat4 model;
mat4 projection;

const int vNum = 6 * 256 * 256;
GLuint vertexArray;
vector<vec3> terrain(0);
GLuint terrainID;
GLuint programID;
GLuint MVPID;
GLuint framebufferVAO;
GLuint heightMapID;


void update_matrix_stack(const mat4 &_model) {
	model = _model;

	projection = Eigen::perspective(45.0f, 4.0f / 3.0f, 0.1f, 15.0f);

	vec3 cam_pos(0.0f, 0.0f, 0.5f);
	vec3 cam_look(0.0f, 0.0f, 0.0f);
	vec3 cam_up(0.0f, 1.0f, 0.0f);
	view = Eigen::lookAt(cam_pos, cam_look, cam_up);
}

void genMap() {
	float increment = 1.0 / 256.0;
	float current_incr_i = 0.0;
	float current_incr_j = 0.0;

	for (int i = 0; i < 256; i++) {

		vec3 low_left = vec3(current_incr_i - 0.5, -0.5, 0.0);
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
		for (int j = 1; j < 256; j++) {

			vec3 low_left = vec3(current_incr_i - 0.5, current_incr_j - 0.5, 0.0);
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

	glGenBuffers(1, &terrainID);
	glBindBuffer(GL_ARRAY_BUFFER, terrainID);
	glEnableVertexAttribArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBufferData(GL_ARRAY_BUFFER, vNum*sizeof(vec3), &(terrain[0]), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);

}

void textureGeneration() {

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	heightMapID = compile_shaders(heightMapGen_vshader, heightMapGen_fshader);
	glUseProgram(heightMapID);

	GLuint frameBufferObject;
	glGenFramebuffers(1, &frameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

	glActiveTexture(GL_TEXTURE0);
	GLuint noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE1);
	GLuint permTableTexture;
	glGenTextures(1, &permTableTexture);
	glBindTexture(GL_TEXTURE_1D, permTableTexture);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RED, GL_UNSIGNED_BYTE, &permTable[0]);
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
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_FLOAT, &gradTable[0]);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	GLuint gradTableLoc = glGetUniformLocation(heightMapID, "gradTableTexture");
	glUniform1i(gradTableLoc, 2);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, noiseTexture, 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	update_matrix_stack(mat4::Identity());

	glGenVertexArrays(1, &framebufferVAO);
	glBindVertexArray(framebufferVAO);

	vec3 quad[4] = {
		vec3(-1.0, -1.0, 0.0),
		vec3(-1.0, 1.0, 0.0),
		vec3(1.0, -1.0, 0.0),
		vec3(1.0, 1.0, 0.0),
	};

	GLuint quadID;
	glGenBuffers(1, &quadID);
	glBindBuffer(GL_ARRAY_BUFFER, quadID);
	GLuint position = glGetAttribLocation(heightMapID, "position");
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), &quad[0], GL_STATIC_DRAW);
	glVertexAttribPointer(position, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
	glEnableVertexAttribArray(position);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MVPID = glGetUniformLocation(heightMapID, "MVP");
	mat4 MVP = projection * view * model;
	glUniformMatrix4fv(MVPID, 1, GL_FALSE, MVP.data());
	//glDrawArrays(GL_TRIANGLES, 0, vNum);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



void init() {

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	update_matrix_stack(mat4::Identity());

	programID = compile_shaders(heightMapGen_vshader, heightMapGen_fshader);
	glUseProgram(programID);
	srand(84654125);
	for (int i = 0; i < 255; i++) {
		permTable[i] = i;
	}
	for (int i = 0; i < 255; i++) {
		float r1 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
		float r2 = rand() / ((double)RAND_MAX / 2.0) - 1.0;
		int t = rand() % 256;
		int tmp = permTable[i];
		permTable[i] = permTable[t];
		permTable[t] = tmp;
		int e = rand() % 2 ? -1 : 1;
		gradTable[i] = vec3(r1, e * sqrt(1 - r1*r1), 0.0);
	}
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
	//genMap();
	glfwTrackball(update_matrix_stack);
	glfwMainLoop();
	return EXIT_SUCCESS;
}
