#version 450

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_col;

layout(location = 0) out vec4 frag_col;

void main()
{
	gl_Position = in_pos;
	frag_col = in_col;
}