#include "material.hh"

#include "../log.hh"
#include "../math/mat4.hh"
#include "assets/model.hh"
#include "enum_string_helper.hh"
#include "instance.hh"

#include <array.hh>
#include <stdlib.h>
#include <yyjson.h>

namespace vkb::vk
{
	namespace
	{
		void read_struct(yyjson_val* j_fields, property_layout& obj)
		{
			yyjson_arr_iter j_fields_iter;
			yyjson_arr_iter_init(j_fields, &j_fields_iter);
			yyjson_val* j_field;

			while ((j_field = yyjson_arr_iter_next(&j_fields_iter)))
			{
				mc::string  name(yyjson_get_str(yyjson_obj_get(j_field, "name")));
				yyjson_val* j_binding = yyjson_obj_get(j_field, "binding");
				j_field = yyjson_obj_get(j_field, "type");
				char const* type_str = yyjson_get_str(yyjson_obj_get(j_field, "kind"));
				if (strcmp(type_str, "struct") == 0)
				{
					property_layout child_obj {property_layout::type::object};
					child_obj.name = static_cast<mc::string&&>(name);
					read_struct(yyjson_obj_get(j_field, "fields"), child_obj);
					obj.object.props.emplace_back(
						static_cast<property_layout&&>(child_obj));
				}
				else if (strcmp(type_str, "matrix") == 0)
				{
					yyjson_val* j_mat_type = yyjson_obj_get(j_field, "elementType");
					if (yyjson_get_uint(yyjson_obj_get(j_field, "rowCount")) == 4 &&
					    yyjson_get_uint(yyjson_obj_get(j_field, "columnCount")) == 4 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_mat_type, "kind")),
					           "scalar") == 0 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_mat_type, "scalarType")),
					           "float32") == 0)
					{
						property_layout child_mat {property_layout::type::simple};
						child_mat.name = static_cast<mc::string&&>(name);
						child_mat.simple.type = property_layout::simple_type::float44;
						obj.object.props.emplace_back(
							static_cast<property_layout&&>(child_mat));
					}
				}
				else if (strcmp(type_str, "resource") == 0)
				{
					property_layout child_res {property_layout::type::resource};
					child_res.name = static_cast<mc::string&&>(name);

					char const* res_type_str =
						yyjson_get_str(yyjson_obj_get(j_field, "baseShape"));
					if (strcmp(res_type_str, "texture2D") == 0)
						child_res.resource.type =
							property_layout::resource_type::texture2D;

					yyjson_val* j_resource_result = yyjson_obj_get(j_field, "resultType");
					yyjson_val* j_type = yyjson_obj_get(j_resource_result, "elementType");
					if (yyjson_get_uint(
							yyjson_obj_get(j_resource_result, "elementCount")) == 4 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_type, "kind")),
					           "scalar") == 0 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_type, "scalarType")),
					           "float32") == 0)
					{
						child_res.resource.format =
							property_layout::resource_format::float4;
					}

					child_res.resource.binding =
						yyjson_get_uint(yyjson_obj_get(j_binding, "index"));
					obj.object.props.emplace_back(
						static_cast<property_layout&&>(child_res));
				}
				else if (strcmp(type_str, "samplerState") == 0)
				{
					property_layout child_sampler {property_layout::type::sampler};
					child_sampler.sampler.binding =
						yyjson_get_uint(yyjson_obj_get(j_binding, "index"));
					obj.object.props.emplace_back(
						static_cast<property_layout&&>(child_sampler));
				}
				else
				{
					// TODO
				}
			}
		}

		void read_param(yyjson_val* j_param, mc::vector<set_layout>& sets)
		{
			yyjson_val* j_binding = yyjson_obj_get(j_param, "binding");
			if (strcmp(yyjson_get_str(yyjson_obj_get(j_binding, "kind")),
			           "subElementRegisterSpace") != 0)
				return;

			uint32_t index = yyjson_get_uint(yyjson_obj_get(j_binding, "index"));
			if (sets.size() < index)
				sets.resize(index);

			yyjson_val* type = yyjson_obj_get(j_param, "type");

			// TODO support more sets kind. Currently only supporting ParameterBlock
			// containing struct.
			if (strcmp(yyjson_get_str(yyjson_obj_get(type, "kind")), "parameterBlock") !=
			    0)
				return;

			yyjson_val* j_layout =
				yyjson_obj_get(yyjson_obj_get(type, "elementVarLayout"), "type");
			if (strcmp(yyjson_get_str(yyjson_obj_get(j_layout, "kind")), "struct") != 0)
				return;

			property_layout obj {property_layout::type::object};
			obj.name = yyjson_get_str(yyjson_obj_get(j_param, "name"));

			yyjson_val* j_bindings =
				yyjson_obj_get(yyjson_obj_get(type, "elementVarLayout"), "bindings");

			uint32_t binding_off {0};
			// TODO Can uniform data be split im multiple parts without attributes ?
			uint32_t uniform_off {0};
			uint32_t uniform_size {0};

			yyjson_arr_iter elem_bindings_iter;
			yyjson_val*     j_elem_binding;
			yyjson_arr_iter_init(j_bindings, &elem_bindings_iter);
			while ((j_elem_binding = yyjson_arr_iter_next(&elem_bindings_iter)))
			{
				if (strcmp(yyjson_get_str(yyjson_obj_get(j_elem_binding, "kind")),
				           "descriptorTableSlot") == 0)
				{
					binding_off =
						yyjson_get_uint(yyjson_obj_get(j_elem_binding, "index"));
				}
				else if (strcmp(yyjson_get_str(yyjson_obj_get(j_elem_binding, "kind")),
				                "uniform") == 0)
				{
					uniform_off =
						yyjson_get_uint(yyjson_obj_get(j_elem_binding, "offset"));
					uniform_size =
						yyjson_get_uint(yyjson_obj_get(j_elem_binding, "size"));
				}
			}

			read_struct(yyjson_obj_get(j_layout, "fields"), obj);

			sets.emplace_back(static_cast<property_layout&&>(obj), uniform_off,
			                  uniform_size, binding_off);
		}

		void read_entry_point(yyjson_val*                     j_entry_point,
		                      mc::vector<entry_point_layout>& entry_points)
		{
			entry_point_layout entry_layout;
			entry_layout.name = yyjson_get_str(yyjson_obj_get(j_entry_point, "name"));

			char const* stage_str =
				yyjson_get_str(yyjson_obj_get(j_entry_point, "stage"));

			for (uint32_t i {0}; i < mc::to_underlying(shader_stage::count); ++i)
			{
				if (shader_stage_names[i] == stage_str)
				{
					entry_layout.stage = static_cast<shader_stage>(i);
					break;
				}
			}

			if (entry_layout.stage == shader_stage::count)
				log::error("Unknown shader stage '%s'", stage_str);

			entry_points.emplace_back(static_cast<entry_point_layout&&>(entry_layout));
		}

		void fill_bindings(property_layout& obj, uint32_t binding_off,
		                   mc::vector<VkDescriptorSetLayoutBinding>& bindings)
		{
			for (uint32_t i {0}; i < obj.object.props.size(); ++i)
			{
				property_layout& prop = obj.object.props[i];

				switch (prop.type)
				{
					case property_layout::type::simple: break;
					case property_layout::type::resource:
					{
						VkDescriptorSetLayoutBinding binding {};
						binding.descriptorCount = 1;
						binding.binding = binding_off + prop.resource.binding;
						// TODO find a way to be more precise ?
						binding.stageFlags = VK_SHADER_STAGE_ALL;
						switch (prop.resource.type)
						{
							case property_layout::resource_type::texture2D:
							{
								binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
								break;
							}

							default:
								log::assert(false, "Resource type not handled");
								break;
						}

						bindings.emplace_back(binding);

						break;
					}

					case property_layout::type::sampler:
					{
						VkDescriptorSetLayoutBinding binding {};
						binding.descriptorCount = 1;
						binding.binding = binding_off + prop.sampler.binding;
						// TODO find a way to be more precise ?
						binding.stageFlags = VK_SHADER_STAGE_ALL;
						binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

						bindings.emplace_back(binding);
						break;
					}

					case property_layout::type::object:
					{
						fill_bindings(prop, binding_off, bindings);
						break;
					}

					default: log::assert(false, "Property type not handled"); break;
				}
			}
		}
	}

	material::material(mc::string_view shader)
	: path_ {shader}
	{
		FILE* file = fopen(path_.data(), "rb");
		log::assert(file != nullptr, "Invalid shader path: %s", path_.data());
		if (file)
			fclose(file);

		mc::string reflect_path;
		reflect_path.reserve(path_.size() + 5);
		reflect_path += path_;
		reflect_path += ".json";

		yyjson_read_err err;
		yyjson_doc*     doc = yyjson_read_file(reflect_path.data(), 0, nullptr, &err);
		if (!doc)
		{
			log::error("Failed to read shader reflection: %s at %ju", err.msg, err.pos);
			return;
		}

		yyjson_val* j_root = yyjson_doc_get_root(doc);

		yyjson_val* j_params = yyjson_obj_get(j_root, "parameters");
		layout_.sets.reserve(yyjson_arr_size(j_params));
		yyjson_arr_iter j_params_iter;
		yyjson_arr_iter_init(j_params, &j_params_iter);
		yyjson_val* j_param;
		while ((j_param = yyjson_arr_iter_next(&j_params_iter)))
			read_param(j_param, layout_.sets);

		yyjson_val* j_entry_points = yyjson_obj_get(j_root, "entryPoints");
		layout_.entry_points.reserve(yyjson_arr_size(j_entry_points));
		yyjson_arr_iter j_entry_points_iter;
		yyjson_arr_iter_init(j_entry_points, &j_entry_points_iter);
		yyjson_val* j_entry_point;
		while ((j_entry_point = yyjson_arr_iter_next(&j_entry_points_iter)))
			read_entry_point(j_entry_point, layout_.entry_points);
	}

	bool material::create_pipeline_state()
	{
		instance& inst = instance::get();

		desc_set_layouts_.resize(layout_.sets.size());
		mc::vector<VkDescriptorSetLayoutBinding> bindings;
		for (uint32_t i {0}; i < layout_.sets.size(); ++i)
		{
			bindings.clear();
			fill_bindings(layout_.sets[i].obj, layout_.sets[i].binding_off, bindings);
			if (layout_.sets[i].uniform_size)
			{
				VkDescriptorSetLayoutBinding binding {};
				binding.binding = 0;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				binding.descriptorCount = 1;
				binding.stageFlags = VK_SHADER_STAGE_ALL;

				bindings.emplace_back(binding);
			}

			VkDescriptorSetLayoutCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			create_info.bindingCount = bindings.size();
			create_info.pBindings = bindings.data();

			VkResult res = vkCreateDescriptorSetLayout(inst.get_device(), &create_info,
			                                           nullptr, &desc_set_layouts_[i]);

			if (res != VK_SUCCESS)
			{
				log::error("Failed to create descriptor set layout (%s)",
				           string_VkResult(res));
				return false;
			}
		}

		// TODO either hardcode it, like vertex input, either retrieve it from slang
		// reflection
		VkPushConstantRange cst_range {};
		cst_range.size = sizeof(mat4);
		cst_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipe_layout_info {};
		pipe_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipe_layout_info.setLayoutCount = desc_set_layouts_.size();
		pipe_layout_info.pSetLayouts = desc_set_layouts_.data();
		pipe_layout_info.pushConstantRangeCount = 1;
		pipe_layout_info.pPushConstantRanges = &cst_range;

		VkResult res = vkCreatePipelineLayout(inst.get_device(), &pipe_layout_info,
		                                      nullptr, &pipe_layout_);
		if (res != VK_SUCCESS)
		{
			log::error("Failed to create pipeline layout (%s)", string_VkResult(res));
			return false;
		}

		VkShaderModule shader;
		uint32_t*      shader_buf {nullptr};
		int64_t        shader_size {0};
		FILE*          shader_file {fopen(path_.data(), "rb")};
		if (!shader_file)
		{
			log::error("Invalid shader path: %s", path_.data());
			return false;
		}

		fseek(shader_file, 0, SEEK_END);
		fgetpos(shader_file, &shader_size);

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
		if (res != VK_SUCCESS)
		{
			log::error("Failed to create shader module (%s)", string_VkResult(res));
			return false;
		}

		mc::vector<VkPipelineShaderStageCreateInfo> shader_stages_info(
			layout_.entry_points.size());
		for (uint32_t i {0}; i < shader_stages_info.size(); ++i)
		{
			shader_stages_info[i].sType =
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stages_info[i].pNext = nullptr;
			shader_stages_info[i].flags = 0;
			shader_stages_info[i].pSpecializationInfo = nullptr;
			shader_stages_info[i].module = shader;
			shader_stages_info[i].pName = layout_.entry_points[i].name.data();

			switch (layout_.entry_points[i].stage)
			{
				case shader_stage::vertex:
					shader_stages_info[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case shader_stage::fragment:
					shader_stages_info[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
				default: log::assert(false, "unknown stage"); break;
			}
		}

		// TODO explore batch/instantiated rendering
		VkVertexInputBindingDescription input_binding {};
		input_binding.binding = 0;
		input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		input_binding.stride = sizeof(model::vert);

		// TODO hardcode this in a better place, to use it more globally
		// TODO use pos/normal/uv format when importing models
		mc::array<VkVertexInputAttributeDescription, 3> input_attributes;
		input_attributes[0].binding = 0;
		input_attributes[0].location = 0;
		input_attributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		input_attributes[0].offset = offsetof(model::vert, pos);

		input_attributes[1].binding = 0;
		input_attributes[1].location = 1;
		input_attributes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		input_attributes[1].offset = offsetof(model::vert, col);

		input_attributes[2].binding = 0;
		input_attributes[2].location = 2;
		input_attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
		input_attributes[2].offset = offsetof(model::vert, uv);

		VkPipelineVertexInputStateCreateInfo vert_input_info {};
		vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_input_info.vertexBindingDescriptionCount = 1;
		vert_input_info.pVertexBindingDescriptions = &input_binding;
		vert_input_info.vertexAttributeDescriptionCount = input_attributes.size();
		vert_input_info.pVertexAttributeDescriptions = input_attributes.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly {};
		input_assembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// TODO explore more dynamic states to limit PSOs
		VkDynamicState dynamic_states[] {VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
		                                 VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT};
		VkPipelineDynamicStateCreateInfo dynamic_state_info {};
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
		// TODO explore stencil usages (picking, see-through, ...)
		depth_stencil.stencilTestEnable = VK_FALSE;

		VkGraphicsPipelineCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		create_info.stageCount = shader_stages_info.size();
		create_info.pStages = shader_stages_info.data();
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

		vkDestroyShaderModule(inst.get_device(), shader, nullptr);

		if (res != VK_SUCCESS)
		{
			log::error("Failed to create graphics pipeline (%s)", string_VkResult(res));
			return false;
		}

		return true;
	}

	VkDescriptorSetLayout material::get_descriptor_set_layout()
	{
		return desc_set_layouts_[0];
	}

	VkPipelineLayout material::get_pipeline_layout()
	{
		return pipe_layout_;
	}

	VkPipeline material::get_pipeline()
	{
		return pipe_;
	}
} // namespace vkb::vk