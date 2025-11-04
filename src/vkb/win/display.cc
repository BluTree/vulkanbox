#include "display.hh"

namespace vkb
{
	display* display::instance_ {nullptr};

	display& display::get()
	{
		return *instance_;
	}
}