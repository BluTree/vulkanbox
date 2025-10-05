#include "cam/free.hh"
#include "core/time.hh"
#include "input/input_system.hh"
#include "ui/context.hh"
#include "vk/context.hh"
#include "vk/instance.hh"
#include "vk/material.hh"
#include "vk/material/sky_sphere.hh"
#include "vk/surface.hh"
#include "win/window.hh"

#include "log.hh"
#include "math/trig.hh"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_SUPERLUMINAL
#include <Superluminal/PerformanceAPI.h>
#endif

int main(int argc, char** argv)
{
	bool enable_validation = false;
	if (argc > 1)
		enable_validation = strcmp(argv[1], "--validate") == 0;
	using namespace vkb;
	input_system is;
	window       main_window(&is);

	vk::instance inst(enable_validation);
	vk::surface  surface(main_window);

	inst.create_device(surface);
	surface.create_swapchain();

	vk::context ctx(main_window, surface);

	cam::free   cam(is, main_window);
	ui::context ui_ctx(main_window, is, ctx, cam);

	vk::sky_sphere sky;

	bool running {ctx.created()};
	// vkb::log::assert(running, "Failed to initialize Vulkan context");
	time::stamp last = time::now();

	vk::model::vert verts[] {
		// upper face
		{{-1.0f, 1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f, 1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		// bottom face
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-1.0f, 1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f},   {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		// front face
		{{-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, -1.0f, 1.0f, 1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		// back face
		{{-1.0f, 1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f},   {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{-1.0f, 1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		// left face
		{{-1.0f, 1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{-1.0f, 1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		// right face
		{{1.0f, -1.0f, 1.0f, 1.0f},   {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{1.0f, 1.0f, 1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{1.0f, -1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f, 1.0f},   {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	};
	uint16_t idcs[] {
		0,  2,  1,  2,  3,  1,  // upper face
		4,  6,  5,  6,  7,  5,  // bottom face
		8,  10, 9,  10, 11, 9,  // front face
		12, 14, 13, 14, 15, 13, // back face
		16, 18, 17, 18, 19, 17, // left face
		20, 22, 21, 22, 23, 21, // right face
	};

	ctx.set_proj(0.1f, 1000.f, 70.f);

	mc::vector<vk::object> objs;
	objs.reserve(3000);

	srand(0);

	vk::model   model;
	vk::texture tex;
	ctx.init_model(model, verts, idcs);
	ctx.init_texture(tex, "res/textures/tex.png");

	for (uint32_t i {0}; i < objs.capacity(); i++)
	{
		vk::object& obj = objs.emplace_back();
		obj.pos = {static_cast<float>(rand() % 100), static_cast<float>(rand() % 100),
		           static_cast<float>(rand() % 100), 1.0f};
		obj.rot_axis =
			vkb::vec4(static_cast<float>(rand() % 100), static_cast<float>(rand() % 100),
		              static_cast<float>(rand() % 100), 1.0f)
				.norm3();
		obj.scale = {.5f, .5f, .5f, 1.f};
		obj.rot_speed = static_cast<float>(rand() % 100) / 50.f;

		obj.model = &model;
		obj.tex = &tex;
		ctx.init_object(&obj);
	}

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
		cam.update(dt);

		for (uint32_t i {0}; i < objs.size(); i++)
			objs[i].update(dt);

		if (!main_window.closed() && !main_window.minimized())
		{
			ui_ctx.update(dt);
			ctx.prepare_draw(cam);
			sky.prepare_draw(ctx.current_command_buffer(), ctx.current_img_idx(), cam,
			                 ctx.get_proj());

			ctx.begin_draw();
			sky.draw(ctx.current_command_buffer(), ctx.current_img_idx());
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

	ctx.destroy_texture(tex);
	ctx.destroy_model(model);

	for (uint32_t i {0}; i < objs.size(); i++)
		ctx.destroy_object(&objs[i]);
	return 0;
}