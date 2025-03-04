#include "core/time.hh"
#include "ui/context.hh"
#include "vk/context.hh"
#include "win/window.hh"

#include "log.hh"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	using namespace vkb;
	window      main_window;
	vk::context ctx(main_window);
	ui::context ui_ctx(main_window, ctx);

	bool running {ctx.created()};
	// vkb::log::assert(running, "Failed to initialize Vulkan context");
	time::stamp last = time::now();

	while (running)
	{
		time::stamp now = time::now();
		double      dt = time::elapsed_sec(last, now);
		last = now;
		main_window.update();
		ui_ctx.update(dt);

		if (!main_window.closed() && !main_window.minimized())
		{
			ctx.begin_draw();
			ctx.draw(dt);
			ui_ctx.draw();
			ctx.present();
		}

		if (main_window.closed())
			running = false;
	}

	ctx.wait_completion();
	return 0;
}