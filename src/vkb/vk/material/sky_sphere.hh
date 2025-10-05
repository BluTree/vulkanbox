#pragma once

#include <vulkan/vulkan.h>

#include "../../cam/free.hh"
#include "../buffer.hh"

namespace vkb::vk
{
	class sky_sphere
	{
	public:
		sky_sphere();
		sky_sphere(sky_sphere const&) = delete;
		sky_sphere(sky_sphere&&) = delete;
		~sky_sphere();

		sky_sphere& operator=(sky_sphere const&) = delete;
		sky_sphere& operator=(sky_sphere&&) = delete;

		void prepare_draw(VkCommandBuffer cmd, uint32_t const img_idx,
		                  cam::free const& cam, mat4 const& proj);
		void draw(VkCommandBuffer cmd, uint32_t const img_idx);

	private:
		VkDescriptorSetLayout desc_set_layout_ {nullptr};
		VkDescriptorPool      desc_pool_ {nullptr};
		VkDescriptorSet       desc_sets_[3] {nullptr};
		buffer                staging_uniforms_[3];
		buffer                uniforms_[3];

		VkPipelineLayout pipe_layout_ {nullptr};
		VkPipeline       pipe_ {nullptr};

		buffer vertices_;
		buffer indices_;
	};
}