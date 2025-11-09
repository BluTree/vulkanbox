#include "sky_sphere.hh"

#include "../../cam/base.hh"
#include "../../log.hh"
#include "../../math/mat4.hh"
#include "../../math/math.hh"
#include "../../sphere.hh"
#include "../enum_string_helper.hh"
#include "../instance.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace vkb::vk
{
	sky_sphere::sky_sphere()
	{
		instance& inst = instance::get();
		VkResult  res = VK_SUCCESS;

		// Descriptor Set
		{
			VkDescriptorSetLayoutBinding binding {};
			binding.binding = 0;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutCreateInfo layout_info {};
			layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout_info.bindingCount = 1;
			layout_info.pBindings = &binding;

			res = vkCreateDescriptorSetLayout(inst.get_device(), &layout_info, nullptr,
			                                  &desc_set_layout_);
			log::assert(res == VK_SUCCESS, "Failed to create descriptor set layout (%s)",
			            string_VkResult(res));

			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			res = vkCreateDescriptorSetLayout(inst.get_device(), &layout_info, nullptr,
			                                  &star_positions_layout_);
			log::assert(res == VK_SUCCESS, "Failed to create descriptor set layout (%s)",
			            string_VkResult(res));

			VkDescriptorPoolSize pool_sizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4}
            };

			VkDescriptorPoolCreateInfo pool_info {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 4;
			pool_info.pPoolSizes = pool_sizes;
			pool_info.poolSizeCount = 1;
			res = vkCreateDescriptorPool(instance::get().get_device(), &pool_info,
			                             nullptr, &desc_pool_);
			log::assert(res == VK_SUCCESS, "Failed to create descriptor pool (%s)",
			            string_VkResult(res));

			VkDescriptorSetLayout       layouts[4] {desc_set_layout_, desc_set_layout_,
			                                        desc_set_layout_, star_positions_layout_};
			VkDescriptorSetAllocateInfo alloc_info {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = desc_pool_;
			alloc_info.descriptorSetCount = 4;
			alloc_info.pSetLayouts = layouts;
			VkDescriptorSet sets[4];
			res = vkAllocateDescriptorSets(inst.get_device(), &alloc_info, sets);
			log::assert(res == VK_SUCCESS, "Failed to create descriptor sets (%s)",
			            string_VkResult(res));

			for (uint32_t i {0}; i < 3; ++i)
			{
				desc_sets_[i] = sets[i];
				staging_uniforms_[i] =
					inst.create_buffer(sizeof(mat4), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

				uniforms_[i] = inst.create_buffer(sizeof(mat4),
				                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				VkDescriptorBufferInfo buf_info {};
				buf_info.buffer = uniforms_[i].buffer;
				buf_info.offset = 0;
				buf_info.range = sizeof(mat4);

				VkWriteDescriptorSet write {};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = desc_sets_[i];
				write.dstBinding = 0;
				write.dstArrayElement = 0;
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.descriptorCount = 1;
				write.pBufferInfo = &buf_info;
				vkUpdateDescriptorSets(inst.get_device(), 1, &write, 0, nullptr);
			}

			star_positions_set_ = sets[3];
			constexpr uint32_t star_count = 1000;

			buffer staging = inst.create_buffer(sizeof(star) * star_count,
			                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			star_positions_uniform_ = inst.create_buffer(
				sizeof(star) * star_count,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkDescriptorBufferInfo buf_info {};
			buf_info.buffer = star_positions_uniform_.buffer;
			buf_info.offset = 0;
			buf_info.range = sizeof(star) * star_count;

			VkWriteDescriptorSet write {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = star_positions_set_;
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &buf_info;
			vkUpdateDescriptorSets(inst.get_device(), 1, &write, 0, nullptr);

			star stars[star_count];
			for (uint32_t i {0}; i < star_count; ++i)
			{
				stars[i].pos = math::generate_sphere_point();

				stars[i].intensity = math::rand() * 0.8 + 0.2;
			}

			void* buf_mem;
			vmaMapMemory(inst.get_allocator(), staging.memory, &buf_mem);
			memcpy(buf_mem, stars, sizeof(stars));
			vmaUnmapMemory(inst.get_allocator(), staging.memory);

			VkCommandBuffer cmd = inst.begin_commands();

			VkBufferCopy2 region {};
			region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
			region.size = sizeof(stars);
			VkCopyBufferInfo2 copy {};
			copy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
			copy.srcBuffer = staging.buffer;
			copy.dstBuffer = star_positions_uniform_.buffer;
			copy.regionCount = 1;
			copy.pRegions = &region;
			vkCmdCopyBuffer2(cmd, &copy);

			inst.end_commands(cmd);

			inst.destroy_buffer(staging);
		}

		// Pipeline
		{
			VkDescriptorSetLayout layouts[] {desc_set_layout_, star_positions_layout_};
			VkPipelineLayoutCreateInfo pipe_layout_info {};
			pipe_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipe_layout_info.setLayoutCount = 2;
			pipe_layout_info.pSetLayouts = layouts;

			res = vkCreatePipelineLayout(inst.get_device(), &pipe_layout_info, nullptr,
			                             &pipe_layout_);
			log::assert(res == VK_SUCCESS, "Failed to create pipeline layout (%s)",
			            string_VkResult(res));

			VkShaderModule shader;
			uint32_t*      shader_buf {nullptr};
			uint32_t       shader_size {0};
			FILE*          shader_file {fopen("res/shaders/sky_sphere.spv", "rb")};

			log::assert(shader_file, "Failed to open sky_sphere.spv");

			fseek(shader_file, 0, SEEK_END);
			shader_size = ftell(shader_file);

			fseek(shader_file, 0, SEEK_SET);
			shader_buf = new uint32_t[shader_size / 4];
			fread(shader_buf, 1, shader_size, shader_file);
			fclose(shader_file);

			VkShaderModuleCreateInfo shader_create_info {};
			shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_create_info.codeSize = shader_size;
			shader_create_info.pCode = shader_buf;
			res = vkCreateShaderModule(instance::get().get_device(), &shader_create_info,
			                           nullptr, &shader);

			delete[] shader_buf;
			log::assert(res == VK_SUCCESS, "Failed to create shader module (%s)",
			            string_VkResult(res));

			VkPipelineShaderStageCreateInfo stages_info[2] {};
			memset(stages_info, 0, sizeof(stages_info));

			stages_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stages_info[0].module = shader;
			stages_info[0].pName = "v_main";
			stages_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

			stages_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stages_info[1].module = shader;
			stages_info[1].pName = "f_main";
			stages_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkVertexInputBindingDescription input_binding {};
			input_binding.binding = 0;
			input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			input_binding.stride = sizeof(vec4);

			VkVertexInputAttributeDescription input_attribute;
			input_attribute.binding = 0;
			input_attribute.location = 0;
			input_attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			input_attribute.offset = 0;

			VkPipelineVertexInputStateCreateInfo vert_input_info {};
			vert_input_info.sType =
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vert_input_info.vertexBindingDescriptionCount = 1;
			vert_input_info.pVertexBindingDescriptions = &input_binding;
			vert_input_info.vertexAttributeDescriptionCount = 1;
			vert_input_info.pVertexAttributeDescriptions = &input_attribute;

			VkPipelineInputAssemblyStateCreateInfo input_assembly {};
			input_assembly.sType =
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			// TODO explore more dynamic states to limit PSOs
			VkDynamicState dynamic_states[] {VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
			                                 VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT};
			VkPipelineDynamicStateCreateInfo dynamic_state_info {};
			dynamic_state_info.sType =
				VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamic_state_info.dynamicStateCount = 2;
			dynamic_state_info.pDynamicStates = dynamic_states;

			VkPipelineViewportStateCreateInfo viewport_state {};
			viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

			VkPipelineRasterizationStateCreateInfo rasterizer {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.f;
			rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			VkPipelineMultisampleStateCreateInfo msaa {};
			msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			msaa.sampleShadingEnable = VK_FALSE;
			msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			msaa.minSampleShading = 1.f;
			msaa.pSampleMask = nullptr;
			msaa.alphaToCoverageEnable = VK_FALSE;
			msaa.alphaToOneEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState color_attachment {};
			color_attachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			color_attachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo color_blend {};
			color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			color_blend.logicOpEnable = VK_FALSE;
			color_blend.logicOp = VK_LOGIC_OP_COPY;
			color_blend.attachmentCount = 1;
			color_blend.pAttachments = &color_attachment;

			VkPipelineDepthStencilStateCreateInfo depth_stencil {};
			depth_stencil.sType =
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil.depthTestEnable = VK_FALSE;
			depth_stencil.depthWriteEnable = VK_FALSE;
			depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depth_stencil.stencilTestEnable = VK_FALSE;

			VkGraphicsPipelineCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			create_info.stageCount = 2;
			create_info.pStages = stages_info;
			create_info.pVertexInputState = &vert_input_info;
			create_info.pInputAssemblyState = &input_assembly;
			create_info.pViewportState = &viewport_state;
			create_info.pRasterizationState = &rasterizer;
			create_info.pMultisampleState = &msaa;
			create_info.pColorBlendState = &color_blend;
			create_info.pDepthStencilState = &depth_stencil;
			create_info.pDynamicState = &dynamic_state_info;
			create_info.layout = pipe_layout_;
			create_info.renderPass = nullptr;
			create_info.subpass = 0;

			VkPipelineRenderingCreateInfo rendering_info {};
			rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
			rendering_info.colorAttachmentCount = 1;
			// TODO use global hardcoded formats
			VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
			rendering_info.pColorAttachmentFormats = &format;
			rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

			create_info.pNext = &rendering_info;

			res = vkCreateGraphicsPipelines(inst.get_device(), nullptr, 1, &create_info,
			                                nullptr, &pipe_);
			log::assert(res == VK_SUCCESS, "Failed to create graphics pipeline (%s)",
			            string_VkResult(res));

			vkDestroyShaderModule(inst.get_device(), shader, nullptr);
		}

		// Model
		{
			buffer vertices_staging = inst.create_buffer(
				sizeof(sphere_vertices), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			buffer indices_staging = inst.create_buffer(
				sizeof(sphere_indices), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			void* buf_mem;
			vmaMapMemory(inst.get_allocator(), vertices_staging.memory, &buf_mem);
			memcpy(buf_mem, &sphere_vertices, sizeof(sphere_vertices));
			vmaUnmapMemory(inst.get_allocator(), vertices_staging.memory);

			vmaMapMemory(inst.get_allocator(), indices_staging.memory, &buf_mem);
			memcpy(buf_mem, &sphere_indices, sizeof(sphere_indices));
			vmaUnmapMemory(inst.get_allocator(), indices_staging.memory);

			vertices_ = inst.create_buffer(sizeof(sphere_vertices),
			                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			indices_ = inst.create_buffer(sizeof(sphere_indices),
			                              VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkCommandBuffer cmd = inst.begin_commands();

			VkBufferCopy2 region {};
			region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
			region.size = sizeof(sphere_vertices);
			VkCopyBufferInfo2 copy {};
			copy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
			copy.srcBuffer = vertices_staging.buffer;
			copy.dstBuffer = vertices_.buffer;
			copy.regionCount = 1;
			copy.pRegions = &region;
			vkCmdCopyBuffer2(cmd, &copy);

			region.size = sizeof(sphere_indices);
			copy.srcBuffer = indices_staging.buffer;
			copy.dstBuffer = indices_.buffer;
			vkCmdCopyBuffer2(cmd, &copy);

			inst.end_commands(cmd);

			inst.destroy_buffer(vertices_staging);
			inst.destroy_buffer(indices_staging);
		}
	}

	sky_sphere::~sky_sphere()
	{
		instance& inst = instance::get();

		inst.destroy_buffer(indices_);
		inst.destroy_buffer(vertices_);

		vkDestroyPipeline(inst.get_device(), pipe_, nullptr);
		vkDestroyPipelineLayout(inst.get_device(), pipe_layout_, nullptr);

		inst.destroy_buffer(star_positions_uniform_);

		for (uint32_t i {0}; i < 3; ++i)
		{
			inst.destroy_buffer(staging_uniforms_[i]);
			inst.destroy_buffer(uniforms_[i]);
		}

		vkDestroyDescriptorPool(inst.get_device(), desc_pool_, nullptr);
		vkDestroyDescriptorSetLayout(inst.get_device(), star_positions_layout_, nullptr);
		vkDestroyDescriptorSetLayout(inst.get_device(), desc_set_layout_, nullptr);
	}

	void sky_sphere::prepare_draw(VkCommandBuffer cmd, uint32_t const img_idx,
	                              cam::base const& cam, mat4 const& proj)
	{
		instance& inst = instance::get();
		mat4      transform = cam.rot_mat() * proj;

		void* buf_mem;
		vmaMapMemory(inst.get_allocator(), staging_uniforms_[img_idx].memory, &buf_mem);
		memcpy(buf_mem, &transform, sizeof(mat4));
		vmaUnmapMemory(inst.get_allocator(), staging_uniforms_[img_idx].memory);

		VkBufferCopy2 region {};
		region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		region.size = sizeof(mat4);
		VkCopyBufferInfo2 copy {};
		copy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
		copy.srcBuffer = staging_uniforms_[img_idx].buffer;
		copy.dstBuffer = uniforms_[img_idx].buffer;
		copy.regionCount = 1;
		copy.pRegions = &region;
		vkCmdCopyBuffer2(cmd, &copy);

		VkBufferMemoryBarrier barrier {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = uniforms_[img_idx].buffer;
		barrier.size = sizeof(mat4);
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1,
		                     &barrier, 0, nullptr);
	}

	void sky_sphere::draw(VkCommandBuffer cmd, uint32_t const img_idx)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_);
		VkDeviceSize offset {0};
		vkCmdBindVertexBuffers(cmd, 0, 1, &vertices_.buffer, &offset);
		vkCmdBindIndexBuffer(cmd, indices_.buffer, 0, VK_INDEX_TYPE_UINT16);

		VkDescriptorSet sets[2] {desc_sets_[img_idx], star_positions_set_};

		VkBindDescriptorSetsInfo set_info {};
		set_info.sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO;
		set_info.descriptorSetCount = 2;
		set_info.pDescriptorSets = sets;
		set_info.layout = pipe_layout_;
		set_info.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		vkCmdBindDescriptorSets2(cmd, &set_info);

		// VkBindDescriptorSetsInfo star_info {};
		// star_info.sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO;
		// star_info.descriptorSetCount = 1;
		// star_info.pDescriptorSets = &star_positions_set_;
		// star_info.layout = pipe_layout_;
		// star_info.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		// vkCmdBindDescriptorSets2(cmd, &star_info);

		vkCmdDrawIndexed(cmd, sizeof(sphere_indices) / sizeof(uint16_t), 1, 0, 0, 0);
	}
} // namespace vkb::vk