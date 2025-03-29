#version 450

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_col;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 frag_col;
layout(location = 1) out vec2 uv;

layout(binding = 0) uniform ubo_ {
	// mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform cst_ {
	mat4 model;
} cst;

void main()
{
	gl_Position = ubo.proj * ubo.view * cst.model * in_pos;
	frag_col = in_col;
	uv = in_uv;
}