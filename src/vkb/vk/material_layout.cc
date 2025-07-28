#include "material_layout.hh"

#include <alloc.hh>
#include <string.h>

namespace vkb::vk
{
	mc::string_view shader_stage_names[mc::to_underlying(shader_stage::count)] {
		"vertex",
		"fragment",
	};

	property_layout::property_layout(enum type t)
	: type {t}
	{
		switch (type)
		{
			case type::simple: memset(&simple, 0, sizeof(simple)); break;
			case type::resource: memset(&resource, 0, sizeof(resource)); break;
			case type::sampler: memset(&sampler, 0, sizeof(sampler)); break;
			case type::object:
			{
				new (&object) decltype(object)();
				break;
			}
		}
	}

	property_layout::property_layout(property_layout&& prop)
	: type {prop.type}
	{
		switch (type)
		{
			case type::simple:
			{
				simple = prop.simple;
				break;
			}
			case type::resource:
			{
				resource = prop.resource;
				break;
			}
			case type::sampler:
			{
				sampler = prop.sampler;
				break;
			}
			case type::object:
			{
				new (&object) decltype(object)();
				object.props =
					static_cast<mc::vector<property_layout>&&>(prop.object.props);
				break;
			}
		}
	}

	property_layout::~property_layout() noexcept
	{
		switch (type)
		{
			case type::object:
			{
				object.props.~vector<property_layout>();
				break;
			}

			default: break;
		}
	}

	property_layout& property_layout::operator=(property_layout&& prop)
	{
		switch (type)
		{
			case type::object:
			{
				object.props.~vector<property_layout>();
				break;
			}

			default: break;
		}

		type = prop.type;

		switch (type)
		{
			case type::simple:
			{
				simple = prop.simple;
				break;
			}
			case type::resource:
			{
				resource = prop.resource;
				break;
			}
			case type::sampler:
			{
				sampler = prop.sampler;
				break;
			}
			case type::object:
			{
				object.props =
					static_cast<mc::vector<property_layout>&&>(prop.object.props);
				break;
			}
		}

		return *this;
	}
}