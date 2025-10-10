#include "math.hh"

#include "quat.hh"
#include "trig.hh"
#include "vec4.hh"

#ifdef VKB_WINDOWS
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdlib.h>
#include <time.h>

namespace vkb::math
{
	void init_random()
	{
		srand(time(nullptr));
	}

	double rand()
	{
		return ::rand() / static_cast<double>(RAND_MAX);
	}

	vec4 generate_sphere_point()
	{
		double hor = ((rand()) * M_PI * 2);
		double ver = (asin(rand()) * 2 - 1) * M_PI;

		return {
			static_cast<float>(sin(hor) * cos(ver)),
			static_cast<float>(sin(hor) * sin(ver)),
			static_cast<float>(cos(hor)),
			1.f,
		};
	}
}