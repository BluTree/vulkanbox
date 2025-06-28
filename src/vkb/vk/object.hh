#pragma once

#include "../math/mat4.hh"
#include "../math/vec2.hh"
#include "../math/vec4.hh"

#include <vulkan/vulkan.h>

#include "assets/model.hh"
#include "assets/texture.hh"

namespace vkb::vk
{
	struct object
	{
		void update(double dt);

		vec4   pos {0.f, 0.f, 0.f, 1.f};
		vec4   scale {1.f, 1.f, 1.f, 1.f};
		mat4   trs;
		vec4   rot_axis {0.f, 0.f, 1.f, 1.f};
		double rot {0.0};
		float  rot_speed {1.0f};

		VkDescriptorSet desc_sets_[3] {nullptr};

		model*   model;
		texture* tex;
	};

}