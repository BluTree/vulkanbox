#version 450

layout(location = 0) in vec3 frag_col;

layout(location = 0) out vec4 out_col;

void main()
{
	out_col = vec4(frag_col, 1.0);
}

