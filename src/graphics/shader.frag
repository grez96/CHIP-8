#version 400 core

in vec3 vert_color;

out vec3 frag_color;

void main()
{
	frag_color = vert_color;
}
