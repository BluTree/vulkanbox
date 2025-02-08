#include "win/window.hh"

#include "log.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	vkb::window main_window;

	bool running {true};

	while (running)
	{
		main_window.update();

		if (main_window.closed())
			running = false;
	}
	return 0;
}