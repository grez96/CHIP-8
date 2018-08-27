#version 400 core

layout (location = 0) in vec2 prog_pos_coords;
layout (location = 1) in vec3 prog_color;

uniform mat4 ortho;

out vec3 vert_color;

void main()
{
	gl_Position = ortho * vec4(prog_pos_coords, 0.0, 1.0);
	vert_color = prog_color;
}
