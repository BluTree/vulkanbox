#include "trig.hh"

#ifdef VKB_WINDOWS
#define _USE_MATH_DEFINES
#endif
#include <math.h>

namespace vkb
{
	float deg(float rad)
	{
		return rad * 180 * M_1_PI;
	}

	float rad(float deg)
	{
		return deg * M_PI / 180.0;
	}
}