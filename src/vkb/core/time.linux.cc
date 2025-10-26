#include "time.hh"

namespace vkb::time
{
	stamp now()
	{
		timespec t;
		clock_gettime(CLOCK_REALTIME, &t);

		return t;
	}

	double elapsed_sec(stamp start, stamp end)
	{
		return static_cast<double>(end.tv_sec - start.tv_sec) +
		       static_cast<double>(end.tv_nsec - start.tv_nsec) * 0.000000001;
	}

	double elapsed_ms(stamp start, stamp end)
	{
		return static_cast<double>(end.tv_sec - start.tv_sec) * 1000.0 +
		       static_cast<double>(end.tv_nsec - start.tv_nsec) * 0.000001;
	}
}