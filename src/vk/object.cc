#include "object.hh"

#ifdef VKB_WINDOWS
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <string.h>

namespace vkb::vk
{
	VkVertexInputBindingDescription object::binding_desc()
	{
		VkVertexInputBindingDescription res {};
		res.binding = 0;
		res.stride = sizeof(vert);
		res.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return res;
	}

	mc::array<VkVertexInputAttributeDescription, 3> object::attribute_descs()
	{
		mc::array<VkVertexInputAttributeDescription, 3> res {};

		res[0].binding = 0;
		res[0].location = 0;
		res[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		res[0].offset = offsetof(vert, pos);

		res[1].binding = 0;
		res[1].location = 1;
		res[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		res[1].offset = offsetof(vert, col);

		res[2].binding = 0;
		res[2].location = 2;
		res[2].format = VK_FORMAT_R32G32_SFLOAT;
		res[2].offset = offsetof(vert, uv);

		return res;
	}

	void object::update(double dt)
	{
		struct
		{
			alignas(16) mat4 view;
			alignas(16) mat4 proj;
		} ubo;

		rot = fmod(rot + dt, M_PI * 2.0);
		model = mat4::translate(pos) * mat4::rotate(vec4 {0.f, 0.f, 1.f, 1.f}, rot) *
		        mat4::scale(scale);
		// ubo.model = model;
		// ubo.view = view;
		// ubo.proj = proj;
		// memcpy(uniform_buffers_map_[cur_frame_], &ubo, sizeof(ubo));

		cur_frame_ = (cur_frame_ + 1) % 2;
	}
}