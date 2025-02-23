#version 450

layout(location = 0) in vec4 frag_col;

layout(location = 0) out vec4 out_col;

void main()
{
	out_col = frag_col;
}

