#include "cam/free.hh"
#include "core/time.hh"
#include "input/input_system.hh"
#include "ui/context.hh"
#include "vk/context.hh"
#include "win/window.hh"

#include "log.hh"
#include "math/trig.hh"

#ifdef USE_SUPERLUMINAL
#include <Superluminal/PerformanceAPI.h>
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	using namespace vkb;
	input_system is;
	window       main_window(&is);
	vk::context  ctx(main_window);

	cam::free   cam(is, main_window);
	ui::context ui_ctx(main_window, is, ctx, cam);

	bool running {ctx.created()};
	// vkb::log::assert(running, "Failed to initialize Vulkan context");
	time::stamp last = time::now();

	vk::object::vert verts[] {
		{{-1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, 1.0f, 0.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f},   {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };
	uint16_t idcs[] {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};

	mc::vector<vk::object> objs(2);
	objs[0].model = mat4::rotate({0.f, 0.f, 1.f, 1.f}, rad(45));
	objs[1].pos = {0.f, 2.f, .5f, 1.f};
	objs[1].scale = {.5f, .5f, .5f, 1.f};

	for (uint32_t i {0}; i < objs.size(); i++)
		ctx.init_object(&objs[i], verts, idcs);

	while (running)
	{
#ifdef USE_SUPERLUMINAL
		PerformanceAPI_BeginEvent("Frame", nullptr, PERFORMANCEAPI_DEFAULT_COLOR);
#endif
		time::stamp now = time::now();
		double      dt = time::elapsed_sec(last, now);
		last = now;

		is.clear_transitions();
		main_window.update();
		ui_ctx.update(dt);
		cam.update(dt);

		for (uint32_t i {0}; i < objs.size(); i++)
			objs[i].update(dt);

		if (!main_window.closed() && !main_window.minimized())
		{
			ctx.begin_draw(cam);
			ctx.draw();
			ui_ctx.draw();
			ctx.present();
		}

		if (main_window.closed())
			running = false;
#ifdef USE_SUPERLUMINAL
		PerformanceAPI_EndEvent();
#endif
	}

	ctx.wait_completion();

	for (uint32_t i {0}; i < objs.size(); i++)
		ctx.destroy_object(&objs[i]);
	return 0;
}