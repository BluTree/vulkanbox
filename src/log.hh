#pragma once

#include <stdarg.h>

namespace vkb::log
{
	[[gnu::format(printf, 1, 2)]]
	void debug(char const* format, ...);
	[[gnu::format(printf, 1, 2)]]
	void info(char const* format, ...);
	[[gnu::format(printf, 1, 2)]]
	void warn(char const* format, ...);
	[[gnu::format(printf, 1, 2)]]
	void error(char const* format, ...);

	[[gnu::format(printf, 2, 3)]]
	void assert(bool cond, char const* format, ...);

	[[gnu::format(printf, 1, 0)]]
	void debug_v(char const* format, va_list args);
	[[gnu::format(printf, 1, 0)]]
	void info_v(char const* format, va_list args);
	[[gnu::format(printf, 1, 0)]]
	void warn_v(char const* format, va_list args);
	[[gnu::format(printf, 1, 0)]]
	void error_v(char const* format, va_list args);

	[[gnu::format(printf, 2, 0)]]
	void assert_v(bool cond, char const* format, va_list args);
}