#pragma once

#include <vulkan/vulkan.h>

#include "../../math/vec2.hh"
#include "../buffer.hh"

namespace vkb
{
	class mat4;

	namespace cam
	{
		class base;
	}
}

namespace vkb::vk
{
	class coordinates
	{
	public:
		coordinates();
		coordinates(coordinates const&) = delete;
		coordinates(coordinates&&) = delete;
		~coordinates();

		coordinates& operator=(coordinates const&) = delete;
		coordinates& operator=(coordinates&&) = delete;

		void prepare_draw(VkCommandBuffer cmd, uint32_t const img_idx,
		                  cam::base const& cam, mat4 const& proj, vec2 translate);
		void draw(VkCommandBuffer cmd, uint32_t const img_idx);

	private:
		VkDescriptorSetLayout dynamic_set_layout_ {nullptr};

		VkDescriptorPool desc_pool_ {nullptr};

		VkDescriptorSet dynamic_sets_[3] {nullptr};
		buffer          staging_uniforms_[3];
		buffer          uniforms_[3];

		VkPipelineLayout pipe_layout_ {nullptr};
		VkPipeline       pipe_ {nullptr};

		buffer vertices_;
		buffer indices_;
	};
}