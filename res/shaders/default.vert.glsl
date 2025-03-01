#version 450

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_col;

layout(location = 0) out vec4 frag_col;

layout(binding = 0) uniform ubo_ {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * in_pos;
	frag_col = in_col;
}