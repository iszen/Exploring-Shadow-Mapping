#include "util.h"
#include "PrecisionTimer.h"
#include <epoxy/gl.h>
#include <GL/freeglut.h>
#include <sstream>
#include <fstream>
#include <cstring>

namespace
{
	static PrecisionTimer m_timer;

	GLuint m_defaultProgram = 0;

	const char *vshader = "#version 130\n in vec3 position; in vec4 color; varying vec4 out_color; void main() {out_color = color; gl_Position = vec4(position, 1.0);} ";
	const char *fshader = "#version 130\n varying vec4 out_color; void main() { gl_FragColor = out_color;}";

	void createDefaultShader()
	{
		auto v = glCreateShader(GL_VERTEX_SHADER);
		auto f = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(v, 1, &vshader, NULL);
		glShaderSource(f, 1, &fshader, NULL);

		glCompileShader(v);
		glCompileShader(f);

		m_defaultProgram = glCreateProgram();
		glAttachShader(m_defaultProgram, v);
		glAttachShader(m_defaultProgram, f);
		glBindAttribLocation(m_defaultProgram, 0, "position");
		glBindAttribLocation(m_defaultProgram, 3, "color");

		glLinkProgram(m_defaultProgram);
	}
}

float getFPS()
{
	return 1.f / m_timer.restart();
}

float getMS()
{
	return m_timer.restart() * 1000.f;
}

std::string loadShader(const std::string &filename)
{
	std::string retVal;
	std::ifstream shaderFile(filename.c_str());
	if (!shaderFile.fail())
	{
		shaderFile.seekg(0, std::ios_base::end);
		std::size_t fileSize = static_cast<std::size_t>(shaderFile.tellg());
		shaderFile.seekg(0, std::ios_base::beg);
		char *buffer = new char[fileSize + 1];
		std::memset(buffer, 0, fileSize + 1);
		shaderFile.read(buffer, fileSize);
		retVal = buffer;
		delete[] buffer;
	}
	return retVal;
}

void useDefaultShader()
{
	if (m_defaultProgram == 0)
		createDefaultShader();
	glUseProgram(m_defaultProgram);
}