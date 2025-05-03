#include "object.hh"

#ifdef VKB_WINDOWS
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <string.h>

namespace vkb::vk
{
	void object::update(double dt)
	{
		rot = fmod(rot + (dt * rot_speed), M_PI * 2.0);
		trs = mat4::translate(pos) * mat4::rotate(rot_axis, rot) * mat4::scale(scale);
	}
}