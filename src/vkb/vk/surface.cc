#include "surface.hh"

#include <vulkan/vulkan_win32.h>
#include <win32/misc.h>

#include "../log.hh"

#include "instance.hh"

namespace vkb::vk
{
	surface::surface(window const& win)
	: win_ {win}
	{
		VkWin32SurfaceCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = win_.native_handle();
		create_info.hinstance = GetModuleHandleW(nullptr);

		VkResult res = vkCreateWin32SurfaceKHR(instance::get().get_instance(),
		                                       &create_info, nullptr, &surface_);
		log::assert(res == VK_SUCCESS, "Failed to create surface");
	}

	surface::~surface()
	{
		destroy_swapchain();

		if (surface_)
			vkDestroySurfaceKHR(instance::get().get_instance(), surface_, nullptr);
	}

	void surface::create_swapchain()
	{
		instance& inst = instance::get();
		swapchain_support_ = query_swapchain_support(inst.get_physical_device());

		VkSurfaceFormatKHR format = choose_swap_format();
		VkPresentModeKHR   present_mode = choose_swap_present_mode();
		VkExtent2D         extent = choose_swap_extent();

		uint32_t img_cnt = swapchain_support_.caps.minImageCount + 1;
		if (swapchain_support_.caps.maxImageCount > 0 &&
		    img_cnt > swapchain_support_.caps.maxImageCount)
			img_cnt = swapchain_support_.caps.maxImageCount;

		VkSwapchainCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface_;
		create_info.minImageCount = img_cnt;
		create_info.imageFormat = format.format;
		create_info.imageColorSpace = format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		instance::queue_indices indices = inst.get_queue_indices();
		uint32_t                family_indices[] {indices.graphics, indices.present};

		if (indices.graphics != indices.present)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = family_indices;
		}
		else
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		create_info.preTransform = swapchain_support_.caps.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = nullptr;

		VkResult res =
			vkCreateSwapchainKHR(inst.get_device(), &create_info, nullptr, &swapchain_);
		log::assert(res == VK_SUCCESS, "Cannot create swapchain");

		swapchain_format_ = format;
		swapchain_extent_ = extent;
		vkGetSwapchainImagesKHR(inst.get_device(), swapchain_, &img_cnt, nullptr);
		swapchain_images_.resize(img_cnt);
		vkGetSwapchainImagesKHR(inst.get_device(), swapchain_, &img_cnt,
		                        swapchain_images_.data());

		swapchain_image_views_.resize(swapchain_images_.size());
		for (uint32_t i {0}; i < swapchain_images_.size(); ++i)
		{
			VkImageViewCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.image = swapchain_images_[i];
			create_info.format = swapchain_format_.format;
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;
			create_info.subresourceRange.levelCount = 1;

			res = vkCreateImageView(inst.get_device(), &create_info, nullptr,
			                        &swapchain_image_views_[i]);
			log::assert(res == VK_SUCCESS, "Cannot create swapchain image view %u", i);
		}

		create_depth_resources();
	}

	void surface::destroy_swapchain()
	{
		instance& inst = instance::get();

		if (depth_stencil_view_)
			vkDestroyImageView(inst.get_device(), depth_stencil_view_, nullptr);

		if (depth_stencil_.image && depth_stencil_.memory)
			vmaDestroyImage(inst.get_allocator(), depth_stencil_.image,
			                depth_stencil_.memory);

		for (uint32_t i {0}; i < swapchain_image_views_.size(); ++i)
			vkDestroyImageView(inst.get_device(), swapchain_image_views_[i], nullptr);

		if (swapchain_)
			vkDestroySwapchainKHR(inst.get_device(), swapchain_, nullptr);
	}

	surface::swapchain_support
	surface::query_swapchain_support(VkPhysicalDevice device) const
	{
		swapchain_support res;
		uint32_t          cnt {0};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &res.caps);

		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &cnt, nullptr);
		if (cnt)
		{
			res.formats.resize(cnt);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &cnt,
			                                     res.formats.data());
		}

		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &cnt, nullptr);
		if (cnt)
		{
			res.present_modes.resize(cnt);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &cnt,
			                                          res.present_modes.data());
		}

		return res;
	}

	bool surface::check_present_queue(VkPhysicalDevice device, uint32_t queue_idx) const
	{
		VkBool32 has_present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_idx, surface_, &has_present);
		return has_present == VK_TRUE;
	}

	VkSwapchainKHR surface::get_swapchain() const
	{
		return swapchain_;
	}

	VkSurfaceFormatKHR surface::get_format() const
	{
		return swapchain_format_;
	}

	VkExtent2D surface::get_extent() const
	{
		return swapchain_extent_;
	}

	mc::vector<VkImage> const& surface::get_images() const
	{
		return swapchain_images_;
	}

	mc::vector<VkImageView> const& surface::get_image_views() const
	{
		return swapchain_image_views_;
	}

	VkImage surface::get_depth_stencil() const
	{
		return depth_stencil_.image;
	}

	VkImageView surface::get_depth_stencil_view() const
	{
		return depth_stencil_view_;
	}

	VkSurfaceFormatKHR surface::choose_swap_format()
	{
		for (uint32_t i {0}; i < swapchain_support_.formats.size(); ++i)
			if (swapchain_support_.formats[i].format == VK_FORMAT_B8G8R8_SRGB &&
			    swapchain_support_.formats[i].colorSpace ==
			        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return swapchain_support_.formats[i];

		return swapchain_support_.formats[0];
	}

	VkPresentModeKHR surface::choose_swap_present_mode()
	{
		for (uint32_t i {0}; i < swapchain_support_.present_modes.size(); ++i)
			if (swapchain_support_.present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
				return swapchain_support_.present_modes[i];

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D surface::choose_swap_extent()
	{
		if (swapchain_support_.caps.currentExtent.width != UINT32_MAX &&
		    swapchain_support_.caps.currentExtent.height != UINT32_MAX)
			return swapchain_support_.caps.currentExtent;
		else
		{
			mc::pair size = win_.size();

			VkExtent2D extent {size.first, size.second};

			if (extent.width < swapchain_support_.caps.minImageExtent.width)
				extent.width = swapchain_support_.caps.minImageExtent.width;
			else if (extent.width > swapchain_support_.caps.maxImageExtent.width)
				extent.width = swapchain_support_.caps.maxImageExtent.width;

			if (extent.height < swapchain_support_.caps.minImageExtent.height)
				extent.height = swapchain_support_.caps.minImageExtent.height;
			else if (extent.height > swapchain_support_.caps.maxImageExtent.height)
				extent.height = swapchain_support_.caps.maxImageExtent.height;

			return extent;
		}
	}

	void surface::create_depth_resources()
	{
		instance& inst = instance::get();
		VkFormat  depth_fmt =
			inst.find_supported_format({VK_FORMAT_D32_SFLOAT}, VK_IMAGE_TILING_OPTIMAL,
		                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		depth_stencil_ = inst.create_image(
			swapchain_extent_.width, swapchain_extent_.height, 1, depth_fmt,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		depth_stencil_view_ = inst.create_image_view(depth_stencil_.image, depth_fmt,
		                                             VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		VkCommandBuffer cmd = inst.begin_commands();

		inst.transition_image_layout(cmd, depth_stencil_.image, VK_IMAGE_LAYOUT_UNDEFINED,
		                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0,
		                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		                             VK_IMAGE_ASPECT_DEPTH_BIT);
		inst.end_commands(cmd);
	}
}