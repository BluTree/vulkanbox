#include "base.hh"

namespace vkb::cam
{
	mat4 base::view_mat() const
	{
		return view_mat_;
	}

	mat4 base::rot_mat() const
	{
		return rot_mat_;
	}
}