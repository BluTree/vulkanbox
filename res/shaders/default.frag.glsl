#version 450

layout(location = 0) in vec4 frag_col;
layout(location = 1) in vec2 uv;

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) out vec4 out_col;

void main()
{
	out_col = frag_col * texture(tex, uv*4);
}

