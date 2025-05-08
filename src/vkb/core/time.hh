#pragma once

#include <stdint.h>

namespace vkb
{
	namespace time
	{
#ifdef VKB_WINDOWS
		using stamp = int64_t;
#endif

		stamp now();

		double elapsed_sec(stamp start, stamp end);
		double elapsed_ms(stamp start, stamp end);
	}
}