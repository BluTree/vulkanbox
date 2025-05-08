#pragma once

#include <string_view.hh>

namespace vk::vkb
{
	class material
	{
		material(mc::string_view vert_shader, mc::string_view frag_shader);
	};
}