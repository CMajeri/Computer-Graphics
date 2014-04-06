// Copyright (C) 2014 - LGG EPFL

#include <iostream>
#include <sstream>
#include "common.h"

#include "vshader1.h"
#include "gshader1.h"
#include "fshader1.h"
#include "vshader2.h"
#include "fshader2.h"
#include "gshader2.h"

using namespace std;
using namespace opengp;

mat4 view;
mat4 model;
mat4 projection;

const int vNum = 6 * 128 * 128;
GLuint vertexArray;
vector<vec3> terrain(0);
GLuint terrainID;
GLuint programID;
GLuint MVPID;


void update_matrix_stack(const mat4 &_model) {
	model = _model;

	projection = Eigen::perspective(45.0f, 4.0f / 3.0f, 0.1f, 15.0f);

	vec3 cam_pos(0.0f, 0.0f, 1.0f);
	vec3 cam_look(0.0f, 0.0f, 0.0f);
	vec3 cam_up(0.0f, 1.0f, 0.0f);
	view = Eigen::lookAt(cam_pos, cam_look, cam_up);
}

void genMap() {
	float increment = 1.0 / 128;
	float current_incr_i = 0.0;
	float current_incr_j = 0.0;

	for (int i = 0; i < 128; i++) {

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
		for (int j = 1; j < 128; j++) {

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

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	MVPID = glGetUniformLocation(programID, "MVP");
	mat4 MVP = projection * view * model;
	glUniformMatrix4fv(MVPID, 1, GL_FALSE, MVP.data());
	glDrawArrays(GL_TRIANGLES, 0, vNum);
}

void init() {
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	update_matrix_stack(mat4::Identity());

	programID = compile_shaders(vshader1, fshader1);
	glUseProgram(programID);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glPointSize(0.5);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

int main(int, char**) {
	glfwInitWindowSize(640, 480);
	glfwCreateWindow("Project 1");
	glfwDisplayFunc(display);
	init();
	genMap();
	glfwTrackball(update_matrix_stack);
	glfwMainLoop();
	return EXIT_SUCCESS;
}
