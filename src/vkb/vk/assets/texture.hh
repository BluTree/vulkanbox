#pragma once

#include "../vma/vma.hh"
#include <vulkan/vulkan.h>

namespace vkb::vk
{
	struct texture
	{
		uint32_t      mip_lvl;
		VkImage       img {nullptr};
		VmaAllocation img_memory {nullptr};
		VkImageView   img_view {nullptr};
		VkSampler     sampler {nullptr};
	};
}