#version 150

in vec3 vertex_position;
in vec3 vertex_normal;

out vec3 fragment_normal;
out vec3 fragment_pos;

uniform mat4 mvp;

void main()
{
	fragment_normal = vertex_normal;
	fragment_pos	= vertex_position;
	gl_Position = mvp * vec4(vertex_position.xy, vertex_position.z, 1);
}