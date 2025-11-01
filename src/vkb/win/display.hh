#pragma once

#include <pair.hh>
#include <string_view.hh>
#include <vector.hh>

#ifdef VKB_LINUX
#include <stdint.h>

struct wl_array;

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
struct wl_pointer;
struct wl_keyboard;
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
		static void wm_base_ping(void* ud, xdg_wm_base* wm_base, uint32_t serial);
		static void seat_name(void* ud, wl_seat* seat, char const* name);
		static void seat_capabilities(void* ud, wl_seat* seat, uint32_t capabilities);

		static void pointer_enter(void* ud, wl_pointer* pointer, uint32_t serial,
		                          wl_surface* surface, int32_t surface_x,
		                          int32_t surface_y);
		static void pointer_leave(void* ud, wl_pointer* pointer, uint32_t serial,
		                          wl_surface* surface);
		static void pointer_motion(void* ud, wl_pointer* pointer, uint32_t time,
		                           int32_t surface_x, int32_t surface_y);
		static void pointer_button(void* ud, wl_pointer* pointer, uint32_t serial,
		                           uint32_t time, uint32_t button, uint32_t state);
		static void pointer_axis(void* ud, wl_pointer* pointer, uint32_t time,
		                         uint32_t axis, int32_t value);

		static void keyboard_keymap(void* ud, wl_keyboard* keyboard, uint32_t format,
		                            int32_t fd, uint32_t size);
		static void keyboard_enter(void* ud, wl_keyboard* keyboard, uint32_t serial,
		                           wl_surface* surface, wl_array* keys);
		static void keyboard_leave(void* ud, wl_keyboard* keyboard, uint32_t serial,
		                           wl_surface* surface);
		static void keyboard_key(void* ud, wl_keyboard* keyboard, uint32_t serial,
		                         uint32_t time, uint32_t key, uint32_t state);
		static void keyboard_modifiers(void* ud, wl_keyboard* keyboard, uint32_t serial,
		                               uint32_t mods_depressed, uint32_t mods_latched,
		                               uint32_t mods_locked, uint32_t group);
		static void keyboard_repeat_info(void* ud, wl_keyboard* keyboard, int32_t rate,
		                                 int32_t delay);

		wl_display*    display_ {nullptr};
		wl_compositor* compositor_ {nullptr};
		wl_shm*        shm_ {nullptr};
		wl_seat*       seat_ {nullptr};
		// mc::string     seat_name_;

		wl_output* output_ {nullptr};

		wl_pointer*  pointer_ {nullptr};
		window*      pointer_window_ {nullptr};
		wl_keyboard* keyboard_ {nullptr};
		window*      keyboard_window_ {nullptr};

		xdg_wm_base*                wm_base_ {nullptr};
		zxdg_decoration_manager_v1* decoration_mgr {nullptr};
		libdecor*                   libdecor_ {nullptr};

		mc::vector<mc::pair<wl_surface*, window*>> surface_to_win_;
#endif
	};
}