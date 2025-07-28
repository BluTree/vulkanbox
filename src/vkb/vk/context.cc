#include "context.hh"
#include "instance.hh"

#include "../cam/free.hh"
#include "../log.hh"
#include "../math/trig.hh"
#include "../win/window.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <win32/misc.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vulkan/vulkan_win32.h>
#include <yyjson.h>

#ifdef VKB_WINDOWS
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace vkb::vk
{
	namespace
	{}

	context::context(window const& win, surface& surface)
	: win_ {win}
	, surface_ {surface}
	, mat_ {"res/shaders/default.spv"}
	{
		auto [w, h] = win_.size();
		proj_ = mat4::persp_proj(near_, far_, w / (float)h, rad(fov_deg_));
		view_ = mat4::look_at({0.f, -2.f, 2.f, 1.f}, vec4(), {0.f, 0.f, 1.f, 1.f});

		created_ = create_desc_set_layout();
		if (!created_)
		{
			log::error("Failed to create descriptor set layout");
			return;
		}

		created_ = create_graphics_pipeline();
		if (!created_)
		{
			log::error("Failed to create graphics pipeline");
			return;
		}

		created_ = create_uniform_buffers();
		if (!created_)
		{
			log::error("Failed to create uniform buffers");
			return;
		}

		create_command_buffers();

		created_ = create_sync_objects();
		if (!created_)
		{
			log::error("Failed to create synchronization objects");
			return;
		}

		created_ = create_descriptor_pool();
		if (!created_)
		{
			log::error("Failed to create descriptor pool");
			return;
		}
	}

	context::~context()
	{
		instance& inst = instance::get();

		vkDeviceWaitIdle(inst.get_device());

		if (depth_img_view_)
			vkDestroyImageView(inst.get_device(), depth_img_view_, nullptr);
		if (depth_img_buf_)
			vmaFreeMemory(inst.get_allocator(), depth_img_buf_);
		if (depth_img_)
			vkDestroyImage(inst.get_device(), depth_img_, nullptr);

		if (desc_pool_)
			vkDestroyDescriptorPool(inst.get_device(), desc_pool_, nullptr);

		for (uint8_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			if (in_flight_fences_[i])
				vkDestroyFence(inst.get_device(), in_flight_fences_[i], nullptr);
			if (draw_end_semaphores_[i])
				vkDestroySemaphore(inst.get_device(), draw_end_semaphores_[i], nullptr);
			if (img_avail_semaphores_[i])
				vkDestroySemaphore(inst.get_device(), img_avail_semaphores_[i], nullptr);

			if (uniform_buffers_[i])
			{
				vmaDestroyBuffer(inst.get_allocator(), uniform_buffers_[i],
				                 uniform_buffers_memory_[i]);
			}

			if (staging_uniform_buffers_[i])
			{
				vmaDestroyBuffer(inst.get_allocator(), staging_uniform_buffers_[i],
				                 staging_uniform_buffers_memory_[i]);
			}
		}

		for (uint32_t i {0}; i < recycled_semaphores_.size(); ++i)
			vkDestroySemaphore(inst.get_device(), recycled_semaphores_[i], nullptr);

		if (pipe_layout_)
			vkDestroyPipelineLayout(inst.get_device(), pipe_layout_, nullptr);
		if (desc_set_layout_)
			vkDestroyDescriptorSetLayout(inst.get_device(), desc_set_layout_, nullptr);

		if (render_pass_)
			vkDestroyRenderPass(inst.get_device(), render_pass_, nullptr);
	}

	bool context::created() const
	{
		return created_;
	}

	void context::set_proj(float near, float far, float fov_deg)
	{
		near_ = near;
		far_ = far;
		fov_deg_ = fov_deg;

		auto [w, h] = win_.size();
		proj_ = mat4::persp_proj(near_, far_, w / (float)h, rad(fov_deg_));
	}

	bool context::init_model(model& model, mc::array_view<model::vert> verts,
	                         mc::array_view<uint16_t> idcs)
	{
		bool init = create_vertex_buffer(model, verts);
		if (!init)
		{
			log::error("Failed to create vertex buffer");
			return init;
		}

		init = create_index_buffer(model, idcs);
		if (!init)
		{
			log::error("Failed to create index buffer");
			return init;
		}

		return init;
	}

	void context::destroy_model(model& model)
	{
		instance& inst = instance::get();

		if (model.index_buffer_)
			vmaDestroyBuffer(inst.get_allocator(), model.index_buffer_,
			                 model.index_buffer_memory_);

		if (model.vertex_buffer_)
			vmaDestroyBuffer(inst.get_allocator(), model.vertex_buffer_,
			                 model.vertex_buffer_memory_);
	}

	bool context::init_texture(texture& tex, mc::string_view path)
	{
		bool init = create_texture_image(tex, path);
		if (!init)
		{
			log::error("Failed to create texture image");
			return init;
		}

		init = create_texture_image_view(tex);
		if (!init)
		{
			log::error("Failed to create texture image view");
			return init;
		}

		init = create_texture_sampler(tex);
		if (!init)
		{
			log::error("Failed to create texture sampler");
			return init;
		}

		return init;
	}

	void context::destroy_texture(texture& tex)
	{
		instance& inst = instance::get();

		if (tex.sampler)
			vkDestroySampler(inst.get_device(), tex.sampler, nullptr);
		if (tex.img_view)
			vkDestroyImageView(inst.get_device(), tex.img_view, nullptr);
		if (tex.img.image && tex.img.memory)
			vmaDestroyImage(inst.get_allocator(), tex.img.image, tex.img.memory);
	}

	bool context::init_object(object* obj)
	{
		bool init = create_descriptor_sets(obj);
		if (!init)
		{
			log::error("Failed to create descriptor sets");
			return init;
		}

		if (init)
			objs_.emplace_back(obj);

		return init;
	}

	void context::destroy_object(object* obj)
	{
		instance& inst = instance::get();

		vkFreeDescriptorSets(inst.get_device(), desc_pool_, 2, obj->desc_sets_);
	}

	void context::begin_draw(cam::free& cam)
	{
		instance& inst = instance::get();

		VkSemaphore new_img_avail_semaphore {nullptr};
		if (recycled_semaphores_.empty())
		{
			VkSemaphoreCreateInfo info {};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			vkCreateSemaphore(inst.get_device(), &info, nullptr,
			                  &new_img_avail_semaphore);
		}
		else
		{
			new_img_avail_semaphore = recycled_semaphores_.back();
			recycled_semaphores_.pop_back();
		}

		VkResult res =
			vkAcquireNextImageKHR(inst.get_device(), surface_.get_swapchain(), UINT64_MAX,
		                          new_img_avail_semaphore, VK_NULL_HANDLE, &img_idx_);

		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
			return;
		}

		vkWaitForFences(inst.get_device(), 1, &in_flight_fences_[img_idx_], VK_TRUE,
		                UINT64_MAX);
		vkResetFences(inst.get_device(), 1, &in_flight_fences_[img_idx_]);

		vkResetCommandBuffer(command_buffers_[img_idx_], 0);

		VkSemaphore old_semaphore = img_avail_semaphores_[img_idx_];
		if (old_semaphore)
			recycled_semaphores_.emplace_back(old_semaphore);
		img_avail_semaphores_[img_idx_] = new_img_avail_semaphore;

		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		res = vkBeginCommandBuffer(command_buffers_[img_idx_], &begin_info);
		if (res != VK_SUCCESS)
			return;

		struct
		{
			alignas(16) mat4 view;
			alignas(16) mat4 proj;
		} ubo;

		ubo.view = cam.view_mat();
		ubo.proj = proj_;

		void* buff_mem;
		vmaMapMemory(inst.get_allocator(), staging_uniform_buffers_memory_[img_idx_],
		             &buff_mem);
		memcpy(buff_mem, &ubo, sizeof(ubo));
		vmaUnmapMemory(inst.get_allocator(), staging_uniform_buffers_memory_[img_idx_]);

		VkBufferCopy2 region {};
		region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		region.size = sizeof(ubo);
		VkCopyBufferInfo2 copy {};
		copy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
		copy.srcBuffer = staging_uniform_buffers_[img_idx_];
		copy.dstBuffer = uniform_buffers_[img_idx_];
		copy.regionCount = 1;
		copy.pRegions = &region;

		vkCmdCopyBuffer2(command_buffers_[img_idx_], &copy);

		VkBufferMemoryBarrier barrier {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = uniform_buffers_[img_idx_];
		barrier.size = sizeof(ubo);
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(command_buffers_[img_idx_], VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1,
		                     &barrier, 0, nullptr);

		inst.transition_image_layout(
			command_buffers_[img_idx_], surface_.get_images()[img_idx_],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			0,                                            // srcAccessMask
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,         // dstAccessMask
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,            // srcStage
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // dstStage
		);

		VkRenderingAttachmentInfo color_attachment {};
		color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		color_attachment.imageView = surface_.get_image_views()[img_idx_];
		color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingAttachmentInfo depth_attachment {};
		depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depth_attachment.clearValue.depthStencil = {1.f, 0};
		depth_attachment.imageView = surface_.get_depth_stencil_view();
		depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkRenderingInfo render_info {};
		render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		render_info.layerCount = 1;
		render_info.colorAttachmentCount = 1;
		render_info.pColorAttachments = &color_attachment;
		render_info.pDepthAttachment = &depth_attachment;
		render_info.renderArea = {
			{0, 0},
            surface_.get_extent()
        };

		vkCmdBeginRendering(command_buffers_[img_idx_], &render_info);

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = surface_.get_extent().width;
		viewport.height = surface_.get_extent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewportWithCount(command_buffers_[img_idx_], 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = {0, 0};
		scissor.extent = surface_.get_extent();
		vkCmdSetScissorWithCount(command_buffers_[img_idx_], 1, &scissor);
	}

	void context::draw()
	{
		vkCmdBindPipeline(command_buffers_[img_idx_], VK_PIPELINE_BIND_POINT_GRAPHICS,
		                  mat_.get_pipeline());

		for (uint32_t i {0}; i < objs_.size(); ++i)
			record_command_buffer(command_buffers_[img_idx_], objs_[i]);
	}

	void context::present()
	{
		instance& inst = instance::get();

		vkCmdEndRendering(command_buffers_[img_idx_]);

		inst.transition_image_layout(
			command_buffers_[img_idx_], surface_.get_images()[img_idx_],
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,          // srcAccessMask
			0,                                             // dstAccessMask
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStage
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT           // dstStage
		);

		vkEndCommandBuffer(command_buffers_[img_idx_]);
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore          sem_wait[] {img_avail_semaphores_[img_idx_]};
		VkPipelineStageFlags stages_wait[] {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = sem_wait;
		submit_info.pWaitDstStageMask = stages_wait;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers_[img_idx_];

		VkSemaphore sem_signal[] {draw_end_semaphores_[img_idx_]};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = sem_signal;

		vkQueueSubmit(inst.get_graphics_queue(), 1, &submit_info,
		              in_flight_fences_[img_idx_]);

		VkPresentInfoKHR present_info {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = sem_signal;

		VkSwapchainKHR swapchains[] {surface_.get_swapchain()};
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &img_idx_;

		VkResult res = vkQueuePresentKHR(inst.get_present_queue(), &present_info);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
			recreate_swapchain();

		cur_frame_ = (cur_frame_ + 1) % context::max_frames_in_flight;
	}

	void context::fill_init_info(ImGui_ImplVulkan_InitInfo& init_info)
	{
		instance& inst = instance::get();
		init_info.Instance = inst.get_instance();
		init_info.PhysicalDevice = inst.get_physical_device();
		init_info.Device = inst.get_device();
		init_info.Queue = inst.get_graphics_queue();
		init_info.PipelineCache = pipe_cache_;
		init_info.DescriptorPool = desc_pool_;
		init_info.RenderPass = nullptr;
		init_info.UseDynamicRendering = true;

		VkPipelineRenderingCreateInfo rendering_info {};
		rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		rendering_info.colorAttachmentCount = 1;
		surface_format_ = surface_.get_format().format;
		rendering_info.pColorAttachmentFormats = &surface_format_;
		rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

		init_info.PipelineRenderingCreateInfo = rendering_info;
		init_info.Subpass = 0;
		init_info.MinImageCount = context::max_frames_in_flight;
		init_info.ImageCount = context::max_frames_in_flight;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
	}

	VkCommandBuffer context::current_command_buffer()
	{
		return command_buffers_[img_idx_];
	}

	void context::wait_completion()
	{
		instance& inst = instance::get();
		vkDeviceWaitIdle(inst.get_device());
	}

	bool context::create_image_view(VkImage& img, VkFormat format,
	                                VkImageAspectFlags flags, uint32_t mip_lvl,
	                                VkImageView& img_view)
	{
		instance&             inst = instance::get();
		VkImageViewCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.image = img;
		create_info.format = format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = flags;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;
		create_info.subresourceRange.levelCount = mip_lvl;

		VkResult res =
			vkCreateImageView(inst.get_device(), &create_info, nullptr, &img_view);

		return res == VK_SUCCESS;
	}

	bool context::create_desc_set_layout()
	{
		VkDescriptorSetLayoutBinding ubo_binding {};
		ubo_binding.binding = 0;
		ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_binding.descriptorCount = 1;
		ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding sampler_binding {};
		sampler_binding.binding = 1;
		sampler_binding.descriptorCount = 1;
		sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding    bindings[] {ubo_binding, sampler_binding};
		VkDescriptorSetLayoutCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = 2;
		create_info.pBindings = bindings;

		VkResult res = vkCreateDescriptorSetLayout(
			instance::get().get_device(), &create_info, nullptr, &desc_set_layout_);
		return res == VK_SUCCESS;
	}

	bool context::create_graphics_pipeline()
	{
		return mat_.create_pipeline_state();
	}

	VkShaderModule context::create_shader(uint8_t* spirv, uint32_t spirv_size)
	{
		VkShaderModuleCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = spirv_size;
		create_info.pCode = reinterpret_cast<uint32_t*>(spirv);

		VkShaderModule shader;
		if (vkCreateShaderModule(instance::get().get_device(), &create_info, nullptr,
		                         &shader) == VK_SUCCESS)
			return shader;
		else
			return nullptr;
	}

	bool context::create_texture_image(texture& tex, mc::string_view path)
	{
		instance& inst = instance::get();

		int32_t  w, h, c;
		uint8_t* pix = stbi_load(path.data(), &w, &h, &c, STBI_rgb_alpha);
		if (!pix)
			return false;

		tex.mip_lvl = floor(log2(w > h ? w : h));
		uint64_t size = w * h * 4;

		VkBuffer      staging_buf;
		VmaAllocation staging_buf_mem;
		create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		              staging_buf, staging_buf_mem);

		void* data;
		vmaMapMemory(inst.get_allocator(), staging_buf_mem, &data);
		memcpy(data, pix, size);
		vmaUnmapMemory(inst.get_allocator(), staging_buf_mem);

		stbi_image_free(pix);

		tex.img = inst.create_image(
			w, h, tex.mip_lvl, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBuffer cmd = inst.begin_commands();

		inst.transition_image_layout(
			cmd, tex.img.image, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, tex.mip_lvl);

		inst.end_commands(cmd);
		copy_buffer_to_image(staging_buf, tex.img.image, w, h);
		generate_mips(tex.img.image, VK_FORMAT_R8G8B8A8_SRGB, w, h, tex.mip_lvl);

		vmaDestroyBuffer(inst.get_allocator(), staging_buf, staging_buf_mem);

		return true;
	}

	bool context::create_texture_image_view(texture& tex)
	{
		return create_image_view(tex.img.image, VK_FORMAT_R8G8B8A8_SRGB,
		                         VK_IMAGE_ASPECT_COLOR_BIT, tex.mip_lvl, tex.img_view);
	}

	bool context::create_texture_sampler(texture& tex)
	{
		instance& inst = instance::get();

		VkSamplerCreateInfo sampler {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = VK_FILTER_NEAREST;
		sampler.minFilter = VK_FILTER_NEAREST;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.minLod = 0.f;
		sampler.maxLod = tex.mip_lvl;
		sampler.mipLodBias = 0.f;

		VkPhysicalDeviceProperties props {};
		vkGetPhysicalDeviceProperties(inst.get_physical_device(), &props);
		sampler.anisotropyEnable = VK_TRUE;
		sampler.maxAnisotropy = props.limits.maxSamplerAnisotropy;

		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		sampler.unnormalizedCoordinates = VK_FALSE;
		sampler.compareEnable = VK_FALSE;
		sampler.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		VkResult res =
			vkCreateSampler(inst.get_device(), &sampler, nullptr, &tex.sampler);
		return res == VK_SUCCESS;
	}

	void context::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t w,
	                                   uint32_t h)
	{
		instance&       inst = instance::get();
		VkCommandBuffer cmd = inst.begin_commands();

		VkBufferImageCopy2 copy {};
		copy.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent.width = w;
		copy.imageExtent.height = h;
		copy.imageExtent.depth = 1;

		VkCopyBufferToImageInfo2 info {};
		info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
		info.srcBuffer = buffer;
		info.dstImage = image;
		info.regionCount = 1;
		info.pRegions = &copy;
		info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdCopyBufferToImage2(cmd, &info);

		inst.end_commands(cmd);
	}

	void context::generate_mips(VkImage img, VkFormat format, uint32_t w, uint32_t h,
	                            uint32_t mip_lvl)
	{
		instance& inst = instance::get();

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(instance::get().get_physical_device(), format,
		                                    &props);
		if (!(props.optimalTilingFeatures &
		      VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			return;
		VkCommandBuffer cmd = inst.begin_commands();

		VkImageMemoryBarrier barrier {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = img;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mip_w = w;
		int32_t mip_h = h;
		for (uint32_t i {1}; i < mip_lvl; ++i)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
			                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
			                     nullptr, 1, &barrier);

			VkImageBlit blit {};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mip_w, mip_h, 1};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = {mip_w > 1 ? mip_w / 2 : 1, mip_h > 1 ? mip_h / 2 : 1,
			                      1};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(cmd, img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, img,
			               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
			               VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
			                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
			                     nullptr, 1, &barrier);

			if (mip_w > 1)
				mip_w /= 2;
			if (mip_h > 1)
				mip_h /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mip_lvl - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
		                     nullptr, 1, &barrier);

		inst.end_commands(cmd);
	}

	bool context::create_vertex_buffer(model& model, mc::array_view<model::vert> verts)
	{
		instance& inst = instance::get();

		uint64_t buf_size {sizeof(model::vert) * verts.size()};

		VkBuffer      staging_buf {nullptr};
		VmaAllocation staging_buf_memory {nullptr};

		bool res = create_buffer(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                         staging_buf, staging_buf_memory);
		if (!res)
			return false;

		void* buff_mem;
		vmaMapMemory(inst.get_allocator(), staging_buf_memory, &buff_mem);
		memcpy(buff_mem, verts.data(), buf_size);
		vmaUnmapMemory(inst.get_allocator(), staging_buf_memory);

		res = create_buffer(buf_size,
		                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model.vertex_buffer_,
		                    model.vertex_buffer_memory_);

		copy_buffer(staging_buf, model.vertex_buffer_, buf_size);
		vmaDestroyBuffer(inst.get_allocator(), staging_buf, staging_buf_memory);

		return res;
	}

	bool context::create_index_buffer(model& model, mc::array_view<uint16_t> idcs)
	{
		instance& inst = instance::get();

		uint64_t buf_size {sizeof(uint16_t) * idcs.size()};

		VkBuffer      staging_buf {nullptr};
		VmaAllocation staging_buf_memory {nullptr};

		bool res = create_buffer(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                         staging_buf, staging_buf_memory);
		if (!res)
			return false;

		void* buff_mem;
		vmaMapMemory(inst.get_allocator(), staging_buf_memory, &buff_mem);
		memcpy(buff_mem, idcs.data(), buf_size);
		vmaUnmapMemory(inst.get_allocator(), staging_buf_memory);

		res = create_buffer(
			buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model.index_buffer_,
			model.index_buffer_memory_);

		copy_buffer(staging_buf, model.index_buffer_, buf_size);

		vmaDestroyBuffer(inst.get_allocator(), staging_buf, staging_buf_memory);

		model.idc_size = idcs.size();

		return res;
	}

	bool context::create_uniform_buffers()
	{
		uint64_t buf_size = 2 * sizeof(mat4);

		for (uint32_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			create_buffer(buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			              staging_uniform_buffers_[i],
			              staging_uniform_buffers_memory_[i]);
			create_buffer(buf_size,
			              VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniform_buffers_[i],
			              uniform_buffers_memory_[i]);
		}

		return true;
	}

	bool context::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
	                            [[maybe_unused]] VkMemoryPropertyFlags props,
	                            VkBuffer& buf, VmaAllocation& buf_mem)
	{
		instance& inst = instance::get();

		VkBufferCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.size = size;
		create_info.usage = usage;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo                 alloc_info {};
		VkPhysicalDeviceMemoryProperties const* mem_props;
		vmaGetMemoryProperties(inst.get_allocator(), &mem_props);
		for (uint32_t i {0}; i < mem_props->memoryTypeCount; ++i)
			if ((mem_props->memoryTypes[i].propertyFlags & props) == props)
				alloc_info.memoryTypeBits = 1 << i;
		VkResult res =
			vmaCreateBuffer(reinterpret_cast<VmaAllocator>(inst.get_allocator()),
		                    &create_info, &alloc_info, &buf, &buf_mem, nullptr);

		return res == VK_SUCCESS;
	}

	void context::copy_buffer(VkBuffer src, VkBuffer dst, uint64_t size)
	{
		instance&       inst = instance::get();
		VkCommandBuffer cmd = inst.begin_commands();

		VkBufferCopy2 region {};
		region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		region.size = size;
		VkCopyBufferInfo2 copy {};
		copy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
		copy.srcBuffer = src;
		copy.dstBuffer = dst;
		copy.regionCount = 1;
		copy.pRegions = &region;

		vkCmdCopyBuffer2(cmd, &copy);

		inst.end_commands(cmd);
	}

	void context::create_command_buffers()
	{
		mc::vector<VkCommandBuffer> cmds =
			instance::get().allocate_commands(context::max_frames_in_flight);

		for (uint32_t i {0}; i < context::max_frames_in_flight; ++i)
			command_buffers_[i] = cmds[i];
	}

	bool context::create_sync_objects()
	{
		instance& inst = instance::get();

		VkSemaphoreCreateInfo sem_info {};
		sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint8_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			VkResult res1 = vkCreateSemaphore(inst.get_device(), &sem_info, nullptr,
			                                  &img_avail_semaphores_[i]);
			VkResult res2 = vkCreateSemaphore(inst.get_device(), &sem_info, nullptr,
			                                  &draw_end_semaphores_[i]);
			VkResult res3 = vkCreateFence(inst.get_device(), &fence_info, nullptr,
			                              &in_flight_fences_[i]);

			if (res1 != VK_SUCCESS && res2 != VK_SUCCESS && res3 != VK_SUCCESS)
				return false;
		}
		return true;
	}

	bool context::create_descriptor_pool()
	{
		// TODO nvidia not coherent with spec. This will surely cause issues
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  1},
			{VK_DESCRIPTOR_TYPE_SAMPLER,        1},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
        };
		VkDescriptorPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = context::max_frames_in_flight * 10001;
		pool_info.pPoolSizes = pool_sizes;
		VkResult res = vkCreateDescriptorPool(instance::get().get_device(), &pool_info,
		                                      nullptr, &desc_pool_);

		return res == VK_SUCCESS;
	}

	bool context::create_descriptor_sets(object* obj)
	{
		instance& inst = instance::get();

		VkDescriptorSetLayout layouts[context::max_frames_in_flight] {
			mat_.get_descriptor_set_layout(), mat_.get_descriptor_set_layout(),
			mat_.get_descriptor_set_layout()};
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = desc_pool_;
		alloc_info.descriptorSetCount = context::max_frames_in_flight;
		alloc_info.pSetLayouts = layouts;

		VkResult res =
			vkAllocateDescriptorSets(inst.get_device(), &alloc_info, obj->desc_sets_);
		if (res != VK_SUCCESS)
			return false;

		for (uint32_t i {0}; i < context::max_frames_in_flight; ++i)
		{
			VkDescriptorBufferInfo buf_info {};
			buf_info.buffer = uniform_buffers_[i];
			buf_info.offset = 0;
			buf_info.range = 2 * sizeof(mat4);

			VkDescriptorImageInfo img_info {};
			img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			img_info.imageView = obj->tex->img_view;
			img_info.sampler = obj->tex->sampler;

			VkWriteDescriptorSet write[3];
			memset(write, 0, 3 * sizeof(VkWriteDescriptorSet));
			write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write[0].dstSet = obj->desc_sets_[i];
			write[0].dstBinding = 0;
			write[0].dstArrayElement = 0;
			write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write[0].descriptorCount = 1;
			write[0].pBufferInfo = &buf_info;

			write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write[1].dstSet = obj->desc_sets_[i];
			write[1].dstBinding = 1;
			write[1].dstArrayElement = 0;
			write[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			write[1].descriptorCount = 1;
			write[1].pImageInfo = &img_info;

			write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write[2].dstSet = obj->desc_sets_[i];
			write[2].dstBinding = 2;
			write[2].dstArrayElement = 0;
			write[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			write[2].descriptorCount = 1;
			write[2].pImageInfo = &img_info;

			vkUpdateDescriptorSets(inst.get_device(), 3, write, 0, nullptr);
		}

		return true;
	}

	void context::record_command_buffer(VkCommandBuffer cmd, object* obj)
	{
		vkCmdPushConstants(cmd, mat_.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
		                   sizeof(mat4), &obj->trs);

		VkBuffer     buffs[] {obj->model->vertex_buffer_};
		VkDeviceSize offsets[] {0};
		vkCmdBindVertexBuffers(cmd, 0, 1, buffs, offsets);
		vkCmdBindIndexBuffer(cmd, obj->model->index_buffer_, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		                        mat_.get_pipeline_layout(), 0, 1,
		                        &obj->desc_sets_[img_idx_], 0, nullptr);

		vkCmdDrawIndexed(cmd, obj->model->idc_size, 1, 0, 0, 0);
	}

	void context::recreate_swapchain()
	{
		vkDeviceWaitIdle(instance::get().get_device());

		surface_.destroy_swapchain();
		surface_.create_swapchain();

		auto [w, h] = surface_.get_extent();
		proj_ = mat4::persp_proj(near_, far_, w / (float)h, rad(fov_deg_));
	}
} // namespace vkb::vk