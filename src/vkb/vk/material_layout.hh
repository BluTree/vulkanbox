#pragma once

#include <enum.hh>
#include <string.hh>
#include <string_view.hh>
#include <vector.hh>

namespace vkb::vk
{
	enum class shader_stage
	{
		vertex,
		fragment,

		count
	};

	extern mc::string_view shader_stage_names[mc::to_underlying(shader_stage::count)];

	struct property_layout
	{
		enum class type
		{
			simple,
			resource,
			sampler,
			object
		};

		enum class simple_type
		{
			float1,
			float2,
			float4,
			float44,
		};

		enum class resource_type
		{
			texture2D
		};
		enum class resource_format
		{
			float4
		};

		property_layout(enum type t);
		property_layout(property_layout const&) = delete;
		property_layout(property_layout&&);
		~property_layout() noexcept;

		property_layout& operator=(property_layout const&) = delete;
		property_layout& operator=(property_layout&&);

		union
		{
			struct
			{
				simple_type type;
			} simple;

			struct
			{
				resource_type   type;
				resource_format format;
				uint32_t        binding;
			} resource;

			struct
			{
				uint32_t binding;
			} sampler;

			struct
			{
				mc::vector<property_layout> props;
			} object;
		};

		mc::string name;
		type       type;
	};

	struct set_layout
	{
		property_layout obj {property_layout::type::object};

		uint32_t uniform_off {0};
		uint32_t uniform_size {0};

		uint32_t binding_off {0};
	};

	struct entry_point_layout
	{
		mc::string                  name;
		shader_stage                stage {shader_stage::count};
		property_layout*            param_in {nullptr};
		property_layout*            param_out {nullptr};
		mc::vector<property_layout> push_constants;
	};

	struct layout
	{
		mc::vector<set_layout>         sets;
		mc::vector<entry_point_layout> entry_points;
	};
}