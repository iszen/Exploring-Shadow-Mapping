#pragma once

#ifndef GL_TEMPLATE_UTIL_H
#define GL_TEMPLATE_UTIL_H

#include <epoxy/gl.h>
#include <GL/freeglut.h>

#include <sstream>

float getFPS();
float getMS();

void useDefaultShader();

std::string loadShader(const std::string &filename);

template<typename T>
inline std::string toString(const T &t)
{
	std::stringstream converter;
	converter << t;
	return converter.str();
}

template<typename T>
inline void showInWindowTitle(const T &what)
{
	glutSetWindowTitle(toString(what).c_str());
}

#endif