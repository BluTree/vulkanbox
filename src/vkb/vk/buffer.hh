#pragma once

#include "vma/vma.hh"
#include <vulkan/vulkan.h>

namespace vkb::vk
{
	struct buffer
	{
		VkBuffer      buffer {nullptr};
		VmaAllocation memory {nullptr};
	};
}