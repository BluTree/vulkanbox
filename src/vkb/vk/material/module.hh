#pragma once

#include <vector.hh>
#include <vulkan/vulkan.h>

#include "../buffer.hh"

#include "../../math/mat4.hh"
#include "../../math/vec4.hh"

namespace vkb
{
	namespace cam
	{
		class base;
	}

	namespace vk
	{
		struct texture;
		struct model;
	}
}

namespace vkb::vk
{
	class module
	{
	public:
		module(texture const& tex);
		module(module const&) = delete;
		module(module&&) = delete;
		~module();

		module& operator=(module const&) = delete;
		module& operator=(module&&) = delete;

		void prepare_draw(VkCommandBuffer cmd, uint32_t const img_idx,
		                  cam::base const& cam, mat4 const& proj);
		void draw(VkCommandBuffer cmd, uint32_t const img_idx, model const& cube,
		          mc::vector<mat4> models);

	private:
		VkDescriptorSetLayout static_set_layout_ {nullptr};
		VkDescriptorSetLayout dynamic_set_layout_ {nullptr};

		VkDescriptorPool desc_pool_ {nullptr};

		VkDescriptorSet static_set_ {nullptr};
		VkDescriptorSet dynamic_sets_[3] {nullptr};
		buffer          staging_uniforms_[3];
		buffer          uniforms_[3];

		VkPipelineLayout pipe_layout_ {nullptr};
		VkPipeline       pipe_ {nullptr};
	};
}