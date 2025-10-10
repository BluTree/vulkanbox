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
		~material();

		bool create_pipeline_state();

		VkDescriptorSetLayout get_descriptor_set_layout();
		VkPipelineLayout      get_pipeline_layout();
		VkPipeline            get_pipeline();

	private:
		mc::string path_;

		layout layout_;

		mc::vector<VkDescriptorSetLayout> desc_set_layouts_;
		VkPipelineLayout                  pipe_layout_ {nullptr};
		VkPipeline                        pipe_ {nullptr};
	};
}