#pragma once

#include <array_view.hh>
#include <vector.hh>

#include "vma/vma.hh"
#include <vulkan/vulkan.h>

#include "buffer.hh"
#include "image.hh"

namespace vkb::vk
{
	class surface;

	class instance
	{
	public:
		struct queue_indices
		{
			uint32_t graphics = UINT32_MAX;
			uint32_t present = UINT32_MAX;
		};

		static instance& get();

		instance(bool enable_validation);
		instance(instance const&) = delete;
		instance(instance&&) = delete;
		~instance();

		instance& operator=(instance const&) = delete;
		instance& operator=(instance&&) = delete;

		void create_device(surface const& surface);

		VkInstance get_instance();

		queue_indices get_queue_indices();

		VkDevice         get_device();
		VkPhysicalDevice get_physical_device();

		VmaAllocator get_allocator();

		VkQueue get_graphics_queue();
		VkQueue get_present_queue();

		VkFormat find_supported_format(mc::array_view<VkFormat> formats,
		                               VkImageTiling tiling, VkFormatFeatureFlags feats);

		image create_image(uint32_t w, uint32_t h, uint32_t mip_lvl, VkFormat format,
		                   VkImageTiling tiling, VkImageUsageFlags usage,
		                   VkMemoryPropertyFlags props);

		VkImageView create_image_view(VkImage img, VkFormat format,
		                              VkImageAspectFlags flags, uint32_t mip_lvl);

		void transition_image_layout(
			VkCommandBuffer cmd, VkImage img, VkImageLayout old_layout,
			VkImageLayout new_layout, VkAccessFlags src_access_mask,
			VkAccessFlags dst_access_mask, VkPipelineStageFlags src_stage,
			VkPipelineStageFlags dst_stage,
			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_lvl = 1);

		buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
		                     VkMemoryPropertyFlags props);
		void   destroy_buffer(buffer const& buf);

		VkCommandBuffer begin_commands();
		void            end_commands(VkCommandBuffer cmd);

		mc::vector<VkCommandBuffer> allocate_commands(uint32_t count);
		void                        free_commands(mc::array_view<VkCommandBuffer> cmds);

	private:
		static instance* instance_;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT      message_level,
			VkDebugUtilsMessageTypeFlagsEXT             message_type,
			VkDebugUtilsMessengerCallbackDataEXT const* callback_data, void* ud);

		bool create_instance(bool enable_validation);
		bool register_debug_callback();
		bool check_validation_layers(mc::array_view<char const*> const& layers);
		bool select_physical_device(surface const& surface);

		queue_indices find_queue_indices(VkPhysicalDevice device, surface const& surface);

		bool create_logical_device();
		bool create_allocator();

		bool create_command_pools();

		VkInstance               inst_ {nullptr};
		VkDebugUtilsMessengerEXT debug_messenger_ {nullptr};

		queue_indices queue_indices_;

		VkPhysicalDevice phys_device_ {nullptr};
		VkDevice         device_ {nullptr};

		VmaAllocator allocator_ {nullptr};

		VkQueue graphics_queue_ {nullptr};
		VkQueue present_queue_ {nullptr};

		VkCommandPool command_pool_ {nullptr};
		// VkCommandPool transient_command_pool_ {nullptr};
	};
}