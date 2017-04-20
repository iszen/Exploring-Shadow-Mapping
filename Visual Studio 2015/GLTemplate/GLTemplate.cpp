#define EIGEN_DONT_VECTORIZE
#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

#include <epoxy/gl.h>
#include <GL/freeglut.h>

#include "util.h"
#include "PrecisionTimer.h"

#ifdef WIN32
#include <epoxy/wgl.h>
#include <tchar.h>
#undef min
#undef max
#endif

#include <Eigen/Eigen>

#include "bunny.h"
#include "floor.h"

#define USE_MATH_DEFINES
#include <algorithm>
#include <ctime>
#include <cmath>
#include <iostream>
#include <vector>

GLuint vertexArray, quad, positionBuffer, normalBuffer, indexBuffer, program;
GLuint framebuffer, depthTex;

Eigen::Matrix4f light_trans;

Eigen::Vector4f light_pos(0.2, 0.2, 0.2, 1);
float light_y_rot = 0.25 * M_PI;
float light_x_rot = -0.12 * M_PI;

Eigen::Vector4f cam_pos(0, 0, 0, 1);
float cam_y_rot = 0;
float cam_x_rot = 0;

int lastMouseX = 0,
lastMouseY = 0;

const float mouseSpeed = 0.5f;
const float moveSpeed = 0.05f;

Eigen::Matrix4f getCameraRotationMatrix(float cam_x_rot, float cam_y_rot)
{
	Eigen::Matrix4f cam_y;
	cam_y.setIdentity();
	cam_y(0, 0) = std::cos(cam_y_rot);
	cam_y(0, 2) = std::sin(cam_y_rot);
	cam_y(2, 0) = -std::sin(cam_y_rot);
	cam_y(2, 2) = std::cos(cam_y_rot);

	Eigen::Matrix4f cam_x;
	cam_x.setIdentity();
	cam_x(1, 1) = std::cos(cam_x_rot);
	cam_x(1, 2) = -std::sin(cam_x_rot);
	cam_x(2, 1) = std::sin(cam_x_rot);
	cam_x(2, 2) = std::cos(cam_x_rot);

	return cam_y * cam_x;
}

Eigen::Matrix4f getCameraMatrix(const Eigen::Vector4f &cam_pos, float cam_x_rot, float cam_y_rot)
{
	Eigen::Matrix4f cam_trans;
	cam_trans.setIdentity();
	cam_trans(0, 3) = cam_pos.x();
	cam_trans(1, 3) = cam_pos.y();
	cam_trans(2, 3) = cam_pos.z();

	return cam_trans * getCameraRotationMatrix(cam_x_rot, cam_y_rot);
}

Eigen::Matrix4f getProjection(float n, float f, float aspect, float FOV)
{
	Eigen::Matrix4f ret;
	ret.setZero();
	float s = 1.f / std::tan(FOV * M_PI / 360.f);
	float n_minus_f = n - f;
	ret(0, 0) = s;
	ret(1, 1) = s * aspect;
	ret(2, 2) = (n + f) / n_minus_f;
	ret(2, 3) = 2 * n * f / n_minus_f;
	ret(3, 2) = -1;

	return ret;
}

GLuint createShader(std::string vertexShader, std::string fragmentShader)
{
	GLuint vertShader,
		fragShader;

	auto vertCode = loadShader(vertexShader);
	auto fragCode = loadShader(fragmentShader);

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto cvc = vertCode.c_str();
	auto cfc = fragCode.c_str();
	glShaderSource(vertShader, 1, &cvc, nullptr);
	glShaderSource(fragShader, 1, &cfc, nullptr);

	glCompileShader(vertShader);
	glCompileShader(fragShader);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glBindAttribLocation(program, 0, "vertex_position");
	glBindAttribLocation(program, 1, "vertex_normal");
	glBindFragDataLocation(program, 0, "finalColor");
	glLinkProgram(program);

	return program;
}

GLuint createTexture(GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data)
{
	GLuint ret = 0;
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
	return ret;
}

void init()
{
	// Bunny
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bunny::vertex_positions), bunny::vertex_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bunny::vertex_normals), bunny::vertex_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bunny::triangle_indices), bunny::triangle_indices, GL_STATIC_DRAW);

	// Bodenplatte
	glGenVertexArrays(1, &quad);
	glBindVertexArray(quad);

	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	GLfloat positions[] = { -100, 0, 100, 100, 0, 100, -100, 0, -100, 100, 0, -100 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	GLfloat normals[] = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);

	// für die "normale" Szene verwendete Shader
	program = createShader("simple.vert", "simple.frag");

	// vorgegebene Lichtprojektion
	light_trans = getProjection(0.1, 100.f, 1.f, 90.f) * getCameraMatrix(light_pos, light_x_rot, light_y_rot).inverse();

	// initiale Kameraposition
	cam_pos = Eigen::Vector4f(0, 0.15, 0.2, 1);

	// Beispielcode zur Erzeugung einer Textur mittels der oben implementierten createTexture Funktion
	// colorTex = createTexture(GL_RGBA8, 1024, 1024, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	depthTex = createTexture(GL_DEPTH_COMPONENT24, 1024, 1024, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);

}

void render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, GL_FALSE, light_trans.data());
	glUniform3fv(glGetUniformLocation(program, "light_pos"), 1, light_pos.data());
	glViewport(0, 0, 1024, 1024);

	// Bunny und Bodenplatte zeichnen
	glBindVertexArray(vertexArray);
	glDrawElements(GL_TRIANGLES, bunny::triangle_indices_size, GL_UNSIGNED_INT, 0);
	glBindVertexArray(quad);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);
	Eigen::Matrix4f pv = getProjection(0.1, 100.f, static_cast<float>(640) / 480, 90.f) * getCameraMatrix(cam_pos, cam_x_rot, cam_y_rot).inverse();
	glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, GL_FALSE, pv.data());
	glUniform3fv(glGetUniformLocation(program, "light_pos"), 1, light_pos.data());
	Eigen::Matrix4f mvp_l = light_trans * pv.inverse();
	glUniformMatrix4fv(glGetUniformLocation(program, "mvp_l"), 1, GL_FALSE, mvp_l.data());
	glUniform1i(glGetUniformLocation(program, "shadowMap"), 2);
	glViewport(0, 0, 640, 480);

	//alles uber indizes
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthTex);

	// Bunny und Bodenplatte zeichnen
	glBindVertexArray(vertexArray);
	glDrawElements(GL_TRIANGLES, bunny::triangle_indices_size, GL_UNSIGNED_INT, 0);
	glBindVertexArray(quad);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glutSwapBuffers();
	showInWindowTitle(getMS());
}

void idle()
{
	glutPostRedisplay();
}


void mouseMotion(int x, int y)
{
	float mouseRADSpeed = mouseSpeed * M_PI / 180.f;

	cam_x_rot += mouseRADSpeed * (lastMouseY - y);
	cam_y_rot += mouseRADSpeed * (lastMouseX - x);

	lastMouseX = x;
	lastMouseY = y;
}

void mousePassiveMotion(int x, int y)
{
	lastMouseX = x;
	lastMouseY = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		cam_pos += getCameraMatrix(cam_pos, cam_x_rot, cam_y_rot) * Eigen::Vector4f(0, 0, -moveSpeed, 0);
		break;
	case 's':
		cam_pos += getCameraMatrix(cam_pos, cam_x_rot, cam_y_rot) * Eigen::Vector4f(0, 0, moveSpeed, 0);
		break;
	case 'a':
		cam_pos += getCameraMatrix(cam_pos, cam_x_rot, cam_y_rot) * Eigen::Vector4f(-moveSpeed, 0, 0, 0);
		break;
	case 'd':
		cam_pos += getCameraMatrix(cam_pos, cam_x_rot, cam_y_rot) * Eigen::Vector4f(moveSpeed, 0, 0, 0);
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int size = 0;
	glutInit(&size, NULL);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitContextVersion(3, 2);

	glutCreateWindow("Hello GL");

	glutDisplayFunc(render);
	glutIdleFunc(idle);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mousePassiveMotion);
	glutKeyboardFunc(keyboardFunc);

	glClearColor(1.f, 0.f, 0.f, 1.f);
	glClearDepth(1.f);

#ifdef WIN32
	wglSwapIntervalEXT(0);
#endif
	
	init();

	glutMainLoop();
	return 0;
}

