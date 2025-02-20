#include "vk/context.hh"
#include "win/window.hh"

#include "log.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	vkb::window      main_window;
	vkb::vk::context ctx(main_window);

	bool running {ctx.created()};
	// vkb::log::assert(running, "Failed to initialize Vulkan context");

	while (running)
	{
		main_window.update();

		ctx.draw();

		if (main_window.closed())
			running = false;
	}
	return 0;
}