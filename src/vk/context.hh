#pragma once

#include <array_view.hh>
#include <string_view.hh>
#include <vector.hh>

#include <vulkan/vulkan.h>

#include <stdint.h>

namespace vkb
{
	class window;
}

namespace vkb::vk
{
	class context
	{
	public:
		context(window const& win);
		~context();

		bool created() const;

		void draw();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT      message_level,
			VkDebugUtilsMessageTypeFlagsEXT             message_type,
			VkDebugUtilsMessengerCallbackDataEXT const* callback_data, void* ud);

		bool create_instance();
		bool register_debug_callback();
		bool check_validation_layers(mc::array_view<char const*> const& layers);

		bool create_surface();

		bool select_physical_device();

		struct queue_families
		{
			uint32_t graphics = UINT32_MAX;
			uint32_t present = UINT32_MAX;
		};

		queue_families find_queue_families(VkPhysicalDevice device);

		struct swapchain_support
		{
			VkSurfaceCapabilitiesKHR       caps;
			mc::vector<VkSurfaceFormatKHR> formats;
			mc::vector<VkPresentModeKHR>   present_modes;
		};

		swapchain_support query_swapchain_support(VkPhysicalDevice device);

		bool create_logical_device();

		VkSurfaceFormatKHR choose_swap_format();
		VkPresentModeKHR   choose_swap_present_mode();
		VkExtent2D         choose_swap_extent();
		bool               create_swapchain();
		bool               create_image_views();

		bool create_render_pass();

		bool create_graphics_pipeline();

		VkShaderModule create_shader(uint8_t* spirv, uint32_t spirv_size);

		bool create_framebuffers();

		bool create_command_pool();
		bool create_command_buffers();

		bool create_sync_objects();

		bool record_command_buffer(VkCommandBuffer cmd, uint32_t img_idx);

		window const& win_;
		bool          created_ {true};

		VkInstance               inst_ {nullptr};
		VkDebugUtilsMessengerEXT debug_messenger_ {nullptr};

		VkSurfaceKHR surface_ {nullptr};

		VkPhysicalDevice  phys_device_ {nullptr};
		queue_families    queue_families_;
		swapchain_support swapchain_support_;
		VkDevice          device_ {nullptr};

		VkQueue graphics_queue_ {nullptr};
		VkQueue present_queue_ {nullptr};

		VkSwapchainKHR          swapchain_;
		VkSurfaceFormatKHR      swapchain_format_;
		VkExtent2D              swapchain_extent_;
		mc::vector<VkImage>     swapchain_images_;
		mc::vector<VkImageView> swapchain_image_views_;

		VkRenderPass render_pass_ {nullptr};

		VkPipelineLayout pipe_layout_ {nullptr};
		VkPipeline       graphics_pipe_ {nullptr};

		mc::vector<VkFramebuffer> framebuffers_;

		VkCommandPool   command_pool_ {nullptr};
		VkCommandBuffer command_buffer_ {nullptr};

		VkSemaphore img_avail_semaphore_ {nullptr};
		VkSemaphore draw_end_semaphore_ {nullptr};
		VkFence     in_flight_fence_ {nullptr};
	};
}