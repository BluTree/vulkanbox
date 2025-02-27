#pragma once

#include "vec4.hh"

namespace vkb
{
	class mat4;

	struct quat
	{
		static quat angle_axis(vec4 axis, float angle);
		static quat euler(vec4 euler);

		vec4 rotate(vec4 vec);

		operator mat4() const;

		float w {0.f};
		float x {0.f};
		float y {0.f};
		float z {0.f};
	};
}