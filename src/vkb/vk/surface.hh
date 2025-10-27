#pragma once

#include <vector.hh>
#include <vulkan/vulkan.h>

#include "../win/window.hh"

#include "image.hh"

namespace vkb::vk
{
	class surface
	{
	public:
		struct swapchain_support
		{
			VkSurfaceCapabilitiesKHR       caps;
			mc::vector<VkSurfaceFormatKHR> formats;
			mc::vector<VkPresentModeKHR>   present_modes;
		};

		surface(window const& win);
		~surface();

		bool need_swapchain_update();
		void create_swapchain();
		void destroy_swapchain();

		swapchain_support query_swapchain_support(VkPhysicalDevice device) const;
		bool check_present_queue(VkPhysicalDevice device, uint32_t queue_idx) const;

		VkSwapchainKHR get_swapchain() const;

		VkSurfaceFormatKHR             get_format() const;
		VkExtent2D                     get_extent() const;
		mc::vector<VkImage> const&     get_images() const;
		mc::vector<VkImageView> const& get_image_views() const;

		VkImage     get_depth_stencil() const;
		VkImageView get_depth_stencil_view() const;

	private:
		VkSurfaceFormatKHR choose_swap_format();
		VkPresentModeKHR   choose_swap_present_mode();
		VkExtent2D         choose_swap_extent();

		void create_depth_resources();

		window const& win_;

		VkSurfaceKHR surface_ {nullptr};

		VkSwapchainKHR          swapchain_;
		swapchain_support       swapchain_support_;
		VkSurfaceFormatKHR      swapchain_format_;
		VkExtent2D              swapchain_extent_;
		mc::vector<VkImage>     swapchain_images_;
		mc::vector<VkImageView> swapchain_image_views_;

		image       depth_stencil_;
		VkImageView depth_stencil_view_ {nullptr};
	};
}