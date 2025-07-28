#pragma once

#include "vma/vma.hh"
#include <vulkan/vulkan.h>

namespace vkb::vk
{
	struct image
	{
		VkImage       image {nullptr};
		VmaAllocation memory {nullptr};
	};
}