#pragma once

namespace vkb::log
{
	void debug(char const* format, ...);
	void info(char const* format, ...);
	void warn(char const* format, ...);
	void error(char const* format, ...);

	void assert(bool cond, char const* format, ...);
}