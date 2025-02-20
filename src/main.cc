#include "vk/context.hh"
#include "win/window.hh"

#include "log.hh"

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	vkb::window      main_window;
	vkb::vk::context ctx(main_window);

	bool running {ctx.created()};
	// vkb::log::assert(running, "Failed to initialize Vulkan context");

	while (running)
	{
		main_window.update();

		if (!main_window.closed() && !main_window.minimized())
			ctx.draw();

		if (main_window.closed())
			running = false;
	}

	return 0;
}