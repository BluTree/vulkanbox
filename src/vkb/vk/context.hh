#pragma once

#include "../math/mat4.hh"
#include "object.hh"

#include "material.hh"
#include "surface.hh"

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
		class base;
	}
}

struct ImGui_ImplVulkan_InitInfo;

namespace vkb::vk
{
	class context
	{
		friend ui::context;

	public:
		context(window const& win, surface& surface);
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

		bool prepare_draw(cam::base& cam);
		void begin_draw();
		void draw();
		void present();

		void fill_init_info(ImGui_ImplVulkan_InitInfo& init_info);

		VkCommandBuffer current_command_buffer();
		uint32_t        current_img_idx();

		void wait_completion();

		mat4 get_proj();

	private:
		constexpr static uint8_t max_frames_in_flight {3};
		uint8_t                  cur_frame_ {0};
		uint32_t                 img_idx_ {0};

		bool create_image_view(VkImage& img, VkFormat format, VkImageAspectFlags flags,
		                       uint32_t mip_lvl, VkImageView& img_view);

		bool create_desc_set_layout();

		bool create_graphics_pipeline();

		VkShaderModule create_shader(uint8_t* spirv, uint32_t spirv_size);

		bool create_texture_image(texture& tex, mc::string_view path);
		bool create_texture_image_view(texture& tex);
		bool create_texture_sampler(texture& tex);
		void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t w, uint32_t h);
		void generate_mips(VkImage img, VkFormat format, uint32_t w, uint32_t h,
		                   uint32_t mip_lvl);
		bool create_vertex_buffer(model& model, mc::array_view<model::vert> verts);
		bool create_index_buffer(model& model, mc::array_view<uint16_t> idcs);
		bool create_uniform_buffers();
		bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
		                   VkMemoryPropertyFlags props, VkBuffer& buf,
		                   VmaAllocation& buf_mem);
		void copy_buffer(VkBuffer src, VkBuffer dst, uint64_t size);

		void create_command_buffers();
		bool create_sync_objects();

		bool create_descriptor_pool();
		bool create_descriptor_sets(object* obj);

		void record_command_buffer(VkCommandBuffer cmd, object* obj);

		void recreate_swapchain();

		window const& win_;
		surface&      surface_;
		bool          created_ {true};

		VkFormat surface_format_;

		material     mat_;
		VkRenderPass render_pass_ {nullptr};

		VkDescriptorSetLayout desc_set_layout_ {nullptr};
		VkPipelineLayout      pipe_layout_ {nullptr};
		// VkPipeline            graphics_pipe_ {nullptr};
		VkPipelineCache pipe_cache_ {VK_NULL_HANDLE};

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