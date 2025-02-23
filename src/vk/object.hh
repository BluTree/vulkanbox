#pragma once

#include "../math/vec2.hh"
#include "../math/vec4.hh"

#include <array.hh>
#include <vector.hh>

#include <vulkan/vulkan.h>

namespace vkb::vk
{
	struct object
	{
		static VkVertexInputBindingDescription                 binding_desc();
		static mc::array<VkVertexInputAttributeDescription, 2> attribute_descs();

		struct vert
		{
			vec4 pos;
			vec4 col;
		};

		mc::vector<vert>     verts;
		mc::vector<uint16_t> idcs;
	};

}