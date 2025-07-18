#pragma once

#include <string.hh>
#include <vector.hh>

namespace vkb::vk
{
	enum class shader_stage
	{
		vertex,
		fragment,

		count
	};

	struct property_layout
	{
		enum class prop_type
		{
			simple,
			resource,
			object
		};

		virtual ~property_layout() = default;
		virtual prop_type get_type() = 0;

		mc::string name;
	};

	struct property_layout_simple : public property_layout
	{
		enum class data_type
		{
			float1,
			float2,
			float4,
			float44,
		};

		property_layout::prop_type get_type() override;

		data_type type;
	};

	struct property_layout_resource : public property_layout
	{
		enum class resource_type
		{
			texture2D
		};
		enum class resource_format
		{
			float4
		};

		property_layout::prop_type get_type() override;

		resource_type   type;
		resource_format format;
		uint32_t        binding;
	};

	struct property_layout_object : public property_layout
	{
		property_layout_object() = default;
		~property_layout_object();
		property_layout::prop_type get_type() override;

		mc::vector<property_layout*> props;
	};

	struct struct_layout
	{
		mc::vector<property_layout*> fields;
	};

	struct set_layout
	{
		~set_layout();

		property_layout_object* obj {nullptr};

		uint32_t uniform_off {0};
		uint32_t uniform_size {0};

		uint32_t binding_off {0};
	};

	struct entry_point_layout
	{
		~entry_point_layout();

		shader_stage                 stage;
		property_layout*             param_in {nullptr};
		property_layout*             param_out {nullptr};
		mc::vector<property_layout*> push_constants;
	};

	struct layout
	{
		mc::vector<set_layout>         sets;
		mc::vector<entry_point_layout> entry_points;
	};
}