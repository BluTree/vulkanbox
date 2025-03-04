#include "time.hh"

#include <win32/misc.h>

namespace vkb::time
{
	namespace
	{
		int64_t get_perf_freq()
		{
			int64_t freq;
			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));

			return freq;
		}

		double const rev_freq {1.0 / get_perf_freq()};
	}

	stamp now()
	{
		int64_t perf_stamp;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&perf_stamp));

		return perf_stamp;
	}

	double elapsed_sec(stamp start, stamp end)
	{
		return static_cast<double>(end - start) * rev_freq;
	}

	double elapsed_ms(stamp start, stamp end)
	{
		return static_cast<double>(end - start) * rev_freq * 1000.0;
	}
}