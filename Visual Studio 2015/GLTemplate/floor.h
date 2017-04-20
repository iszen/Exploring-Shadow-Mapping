#pragma once

#include <epoxy/gl.h>

namespace floor_plane
{
	const GLuint vertex_positions_size = 12;
	const GLfloat vertex_positions[] = { -100, 0, 100, 100, 0, 100, -100, 0, -100, 100, 0, -100 };
	const GLuint vertex_normals_size = 12;
	const GLfloat vertex_normals[] = { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 };
	const GLuint triangle_indices_size = 6;
	const GLuint triangle_indices[] = { 0, 1, 2, 1, 3, 2 };
}