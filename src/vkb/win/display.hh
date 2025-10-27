#pragma once

#include <pair.hh>
#include <string_view.hh>
#include <vector.hh>

#ifdef VKB_LINUX
#include <stdint.h>

struct wl_registry;
struct wl_display;
struct wl_compositor;
struct wl_shm;
struct wl_seat;
struct wl_output;
struct xdg_wm_base;
struct zxdg_decoration_manager_v1;
struct libdecor;
struct wl_surface;
#endif

namespace vkb
{
	class window;
	class input_system;
}

namespace vkb
{
	class display
	{
		friend window;

	public:
		static display& get();

		display();
		display(display const&) = delete;
		display(display&&) = delete;
		~display();

		display& operator=(display const&) = delete;
		display& operator=(display&&) = delete;

		void update();

#ifdef VKB_LINUX
		wl_display*    get_display();
		wl_compositor* get_compositor();
		libdecor*      get_libdecor();
#endif

	private:
		static display* instance_;

		void add_window(window* win);
		void remove_window(window* win);

#ifdef VKB_LINUX
		static void registry_handle(void* ud, wl_registry* registry, uint32_t id,
		                            char const* interface, uint32_t version);
		static void registry_remove(void* ud, wl_registry* registry, uint32_t id);
		static void seat_capabilities(void* ud, wl_seat* seat, uint32_t capabilities);
		static void seat_name(void* ud, wl_seat* seat, char const* name);
		static void wm_base_ping(void* ud, xdg_wm_base* wm_base, uint32_t serial);

		wl_display*    display_ {nullptr};
		wl_compositor* compositor_ {nullptr};
		wl_shm*        shm_ {nullptr};
		wl_seat*       seat_ {nullptr};
		// mc::string     seat_name_;

		wl_output* output_ {nullptr};

		xdg_wm_base*                wm_base_ {nullptr};
		zxdg_decoration_manager_v1* decoration_mgr {nullptr};
		libdecor*                   libdecor_ {nullptr};

		mc::vector<mc::pair<wl_surface*, window*>> surface_to_win_;
#endif
	};
}