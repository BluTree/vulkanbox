#include "material_layout.hh"

namespace vkb::vk
{
	property_layout::prop_type property_layout_simple::get_type()
	{
		return prop_type::simple;
	}

	property_layout::prop_type property_layout_resource::get_type()
	{
		return prop_type::resource;
	}

	property_layout::prop_type property_layout_object::get_type()
	{
		return prop_type::object;
	}

	property_layout_object::~property_layout_object()
	{
		for (uint32_t i {0}; i < props.size(); ++i)
			delete props[i];
	}

	set_layout::~set_layout()
	{
		delete obj;
	}

	entry_point_layout::~entry_point_layout()
	{
		delete param_in;
		delete param_out;

		for (uint32_t i {0}; i < push_constants.size(); ++i)
			delete push_constants[i];
	}
}