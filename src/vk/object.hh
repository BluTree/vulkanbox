#pragma once

#include "../math/mat4.hh"
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
		static mc::array<VkVertexInputAttributeDescription, 3> attribute_descs();

		struct vert
		{
			vec4 pos;
			vec4 col;
			vec2 uv;
		};

		mc::vector<vert>     verts;
		mc::vector<uint16_t> idcs;

		mat4   model;
		double rot {0.0};
	};

}