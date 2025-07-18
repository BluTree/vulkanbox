#pragma once

#include "../vma/vma.hh"
#include <vulkan/vulkan.h>

#include "../image.hh"

namespace vkb::vk
{
	struct texture
	{
		uint32_t    mip_lvl {0};
		image       img;
		VkImageView img_view {nullptr};
		VkSampler   sampler {nullptr};
	};
}