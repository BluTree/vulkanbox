#pragma once

#include "../math/mat4.hh"

namespace vkb::cam
{
	class base
	{
	public:
		mat4 view_mat() const;
		mat4 rot_mat() const;

	protected:
		base() = default;

		mat4 view_mat_;
		mat4 rot_mat_;
	};
}