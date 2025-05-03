#pragma once

#include "../../math/vec2.hh"
#include "../../math/vec4.hh"

#include "../vma/vma.hh"
#include <vulkan/vulkan.h>

#include <array.hh>

namespace vkb::vk
{
	struct model
	{
		struct vert
		{
			vec4 pos;
			vec4 col;
			vec2 uv;
		};

		static VkVertexInputBindingDescription                 binding_desc();
		static mc::array<VkVertexInputAttributeDescription, 3> attribute_descs();

		VkBuffer      vertex_buffer_ {nullptr};
		VmaAllocation vertex_buffer_memory_ {nullptr};
		VkBuffer      index_buffer_ {nullptr};
		VmaAllocation index_buffer_memory_ {nullptr};

		uint32_t idc_size {0};
	};
}