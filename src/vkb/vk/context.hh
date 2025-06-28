#pragma once

#include "../math/mat4.hh"
#include "object.hh"

#include <array_view.hh>
#include <string_view.hh>
#include <vector.hh>

#include "vma/vma.hh"
#include <vulkan/vulkan.h>

#include <stdint.h>

namespace vkb
{
	class window;

	namespace ui
	{
		class context;
	}

	namespace cam
	{
		class free;
	}
}

struct ImGui_ImplVulkan_InitInfo;

namespace vkb::vk
{
	class context
	{
		friend ui::context;

	public:
		context(window const& win, bool enable_validation);
		~context();

		bool created() const;

		void set_proj(float near, float far, float fov_deg);

		bool init_model(model& model, mc::array_view<model::vert> verts,
		                mc::array_view<uint16_t> idcs);
		void destroy_model(model& model);

		bool init_texture(texture& tex, mc::string_view path);
		void destroy_texture(texture& tex);

		bool init_object(object* obj);
		void destroy_object(object* obj);

		void begin_draw(cam::free& cam);
		void draw();
		void present();

		void fill_init_info(ImGui_ImplVulkan_InitInfo& init_info);

		VkCommandBuffer current_command_buffer();

		void wait_completion();

	private:
		constexpr static uint8_t max_frames_in_flight {3};
		uint8_t                  cur_frame_ {0};
		uint32_t                 img_idx_ {0};

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT      message_level,
			VkDebugUtilsMessageTypeFlagsEXT             message_type,
			VkDebugUtilsMessengerCallbackDataEXT const* callback_data, void* ud);

		bool create_instance(bool enable_validation);
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

		bool create_allocator();

		VkSurfaceFormatKHR choose_swap_format();
		VkPresentModeKHR   choose_swap_present_mode();
		VkExtent2D         choose_swap_extent();
		bool               create_swapchain();
		bool               create_image_views();
		bool create_image_view(VkImage& img, VkFormat format, VkImageAspectFlags flags,
		                       uint32_t mip_lvl, VkImageView& img_view);

		bool create_render_pass();

		bool create_desc_set_layout();

		bool create_graphics_pipeline();

		VkShaderModule create_shader(uint8_t* spirv, uint32_t spirv_size);

		bool create_framebuffers();

		bool     create_command_pool();
		bool     create_depth_resources();
		VkFormat find_supported_format(mc::array_view<VkFormat> formats,
		                               VkImageTiling tiling, VkFormatFeatureFlags feats);

		bool create_texture_image(texture& tex, mc::string_view path);
		bool create_texture_image_view(texture& tex);
		bool create_texture_sampler(texture& tex);
		bool create_image(uint32_t w, uint32_t h, uint32_t mip_lvl, VkFormat format,
		                  VkImageTiling tiling, VkImageUsageFlags usage,
		                  VkMemoryPropertyFlags props, VkImage& image,
		                  VmaAllocation& mem);
		void transition_image_layout(VkImage image, VkFormat format,
		                             VkImageLayout old_layout, VkImageLayout new_layout,
		                             uint32_t mip_lvl);
		void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t w, uint32_t h);
		void generate_mips(VkImage img, VkFormat format, uint32_t w, uint32_t h,
		                   uint32_t mip_lvl);
		bool create_vertex_buffer(model& model, mc::array_view<model::vert> verts);
		bool create_index_buffer(model& model, mc::array_view<uint16_t> idcs);
		bool create_uniform_buffers();
		bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
		                   VkMemoryPropertyFlags props, VkBuffer& buf,
		                   VmaAllocation& buf_mem);
		uint32_t        find_mem_type_idx(VkMemoryPropertyFlags props);
		VkCommandBuffer begin_commands();
		void            end_commands(VkCommandBuffer cmd);
		void            copy_buffer(VkBuffer src, VkBuffer dst, uint64_t size);
		bool            create_command_buffers();

		bool create_sync_objects();

		bool create_descriptor_pool();
		bool create_descriptor_sets(object* obj);

		void record_command_buffer(VkCommandBuffer cmd, object* obj);

		void recreate_swapchain();

		window const& win_;
		bool          created_ {true};

		VkInstance               inst_ {nullptr};
		VkDebugUtilsMessengerEXT debug_messenger_ {nullptr};

		VkSurfaceKHR surface_ {nullptr};

		VkPhysicalDevice  phys_device_ {nullptr};
		queue_families    queue_families_;
		swapchain_support swapchain_support_;
		VkDevice          device_ {nullptr};

		// Should be VmaAllocator* but void* to prevent vma inclusion in header
		VmaAllocator allocator_ {nullptr};

		VkQueue graphics_queue_ {nullptr};
		VkQueue present_queue_ {nullptr};

		VkSwapchainKHR          swapchain_;
		VkSurfaceFormatKHR      swapchain_format_;
		VkExtent2D              swapchain_extent_;
		mc::vector<VkImage>     swapchain_images_;
		mc::vector<VkImageView> swapchain_image_views_;

		VkRenderPass render_pass_ {nullptr};

		VkDescriptorSetLayout desc_set_layout_ {nullptr};
		VkPipelineLayout      pipe_layout_ {nullptr};
		VkPipeline            graphics_pipe_ {nullptr};
		VkPipelineCache       pipe_cache_ {VK_NULL_HANDLE};

		mc::vector<VkFramebuffer> framebuffers_;

		VkCommandPool   command_pool_ {nullptr};
		VkCommandBuffer command_buffers_[context::max_frames_in_flight] {nullptr};

		mc::vector<VkSemaphore> recycled_semaphores_;
		VkSemaphore img_avail_semaphores_[context::max_frames_in_flight] {nullptr};
		VkSemaphore draw_end_semaphores_[context::max_frames_in_flight] {nullptr};
		VkFence     in_flight_fences_[context::max_frames_in_flight] {nullptr};

		VkDescriptorPool desc_pool_ {VK_NULL_HANDLE};

		VkBuffer      staging_uniform_buffers_[context::max_frames_in_flight] {nullptr};
		VmaAllocation staging_uniform_buffers_memory_[context::max_frames_in_flight] {
			nullptr};
		VkBuffer      uniform_buffers_[context::max_frames_in_flight] {nullptr};
		VmaAllocation uniform_buffers_memory_[context::max_frames_in_flight] {nullptr};

		mat4  view_;
		mat4  proj_;
		float near_ {0.1f};
		float far_ {100.f};
		float fov_deg_ {70.f};

		mc::vector<object*> objs_;

		VkFormat      depth_fmt_;
		VkImage       depth_img_ {nullptr};
		VmaAllocation depth_img_buf_ {nullptr};
		VkImageView   depth_img_view_ {nullptr};
	};
}