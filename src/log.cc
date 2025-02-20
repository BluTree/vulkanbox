#include "log.hh"

#include <string_view.hh>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED "\x1b[31m"
#define YELLOW "\x1b[33m"
#define MAGENTA "\x1b[35m"
#define DEFAULT "\x1b[0m"

namespace vkb::log
{
	namespace
	{
		void print(char const* prefix, char const* format, va_list args)
		{
			char    formatted[4096] {'\0'};
			int32_t result = vsnprintf(formatted, 4096, format, args);

			if (result > 0)
			{
				fputs(prefix, stdout);
				fputc(' ', stdout);
				fputs(formatted, stdout);
				fputc('\n', stdout);
			}
		}
	}

	void debug(char const* format, ...)
	{
		va_list args;
		va_start(args, format);
		print("[debug]", format, args);
		va_end(args);
	}

	void info(char const* format, ...)
	{
		va_list args;
		va_start(args, format);
		print(MAGENTA "[info] " DEFAULT, format, args);
		va_end(args);
	}

	void warn(char const* format, ...)
	{
		va_list args;
		va_start(args, format);
		print(YELLOW "[warn] " DEFAULT, format, args);
		va_end(args);
	}

	void error(char const* format, ...)
	{
		va_list args;
		va_start(args, format);
		print(RED "[error]" DEFAULT, format, args);
		va_end(args);
	}

	void assert(bool cond, char const* format, ...)
	{
		if (!cond)
		{
			va_list args;
			va_start(args, format);
			print(RED "[assert]" DEFAULT, format, args);
			va_end(args);

			abort();
		}
	}

	void debug_v(char const* format, va_list args)
	{
		print("[debug]", format, args);
	}

	void info_v(char const* format, va_list args)
	{
		print(MAGENTA "[info] " DEFAULT, format, args);
	}

	void warn_v(char const* format, va_list args)
	{
		print(YELLOW "[warn] " DEFAULT, format, args);
	}

	void error_v(char const* format, va_list args)
	{
		print(RED "[error]" DEFAULT, format, args);
	}

	void assert_v(bool cond, char const* format, va_list args)
	{
		if (!cond)
		{
			print(RED "[assert]" DEFAULT, format, args);

			abort();
		}
	}
}
