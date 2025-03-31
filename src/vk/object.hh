#pragma once

#include "../math/mat4.hh"
#include "../math/vec2.hh"
#include "../math/vec4.hh"

#include <array.hh>
#include <vector.hh>

#include <vulkan/vulkan.h>

namespace vkb::vk
{
	struct object
	{
		static VkVertexInputBindingDescription                 binding_desc();
		static mc::array<VkVertexInputAttributeDescription, 3> attribute_descs();

		void update(double dt);

		struct vert
		{
			vec4 pos;
			vec4 col;
			vec2 uv;
		};

		vec4   pos {0.f, 0.f, 0.f, 1.f};
		vec4   scale {1.f, 1.f, 1.f, 1.f};
		mat4   model;
		double rot {0.0};

		uint32_t idc_size {0};

		VkBuffer       vertex_buffer_ {nullptr};
		VkDeviceMemory vertex_buffer_memory_ {nullptr};
		VkBuffer       index_buffer_ {nullptr};
		VkDeviceMemory index_buffer_memory_ {nullptr};

		VkDescriptorSet desc_sets_[2] {nullptr};

		uint32_t       mip_lvl_;
		VkImage        tex_img_ {nullptr};
		VkDeviceMemory tex_img_buf_ {nullptr};
		VkImageView    tex_img_view_ {nullptr};
		VkSampler      tex_sampler_ {nullptr};
	};

}