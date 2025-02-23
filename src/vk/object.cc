#include "object.hh"

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

	mc::array<VkVertexInputAttributeDescription, 2> object::attribute_descs()
	{
		mc::array<VkVertexInputAttributeDescription, 2> res {};

		res[0].binding = 0;
		res[0].location = 0;
		res[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		res[0].offset = offsetof(vert, pos);

		res[1].binding = 0;
		res[1].location = 1;
		res[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		res[1].offset = offsetof(vert, col);

		return res;
	}
}