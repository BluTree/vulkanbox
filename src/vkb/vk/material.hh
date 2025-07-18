#pragma once

#include "material_layout.hh"

#include <string.hh>
#include <string_view.hh>

#include <stdint.h>

#include <vulkan/vulkan.h>

namespace vkb::vk
{
	class material
	{
	public:
		material(mc::string_view shader);

		bool create_pipeline_state();

	private:
		mc::string path_;

		layout layout_;

		[[maybe_unused]] VkDescriptorSetLayout desc_set_layout_ {nullptr};
		[[maybe_unused]] VkPipelineLayout      pipe_layout_ {nullptr};
		[[maybe_unused]] VkPipeline            pipe_ {nullptr};
	};
}