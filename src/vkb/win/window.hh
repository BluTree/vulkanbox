#pragma once

#include <stdint.h>

#ifdef VKB_WINDOWS
#include <win32/window.h>
#elif defined(VKB_LINUX)
#include <wayland-client.h>
#endif

#include <pair.hh>
#include <string.hh>

namespace vkb
{
	class input_system;
}
struct xdg_wm_base;
struct zxdg_decoration_manager_v1;
struct libdecor;
struct wl_surface;
struct libdecor_frame;
struct libdecor_configuration;

namespace vkb
{
	class window
	{
	public:
#ifdef VKB_LINUX
		struct wayland
		{
			wl_display*    display_ {nullptr};
			wl_compositor* compositor_ {nullptr};
			wl_shm*        shm_ {nullptr};
			wl_seat*       seat_ {nullptr};
			mc::string     seat_name_;

			wl_output* output_ {nullptr};

			xdg_wm_base*                wm_base_ {nullptr};
			zxdg_decoration_manager_v1* decoration_mgr {nullptr};
			libdecor*                   libdecor_ {nullptr};
		};

		static wayland const& wayland_context();
#endif
		window(input_system* is = nullptr);
		window(window const&) = delete;
		window(window&&);
		~window();

		window& operator=(window const&) = delete;
		window& operator=(window&&);

		void update();

		bool  closed() const;
		bool  minimized() const;
		void* native_handle() const;

		mc::pair<uint32_t, uint32_t> size() const;
		mc::pair<int32_t, int32_t>   position() const;

		void lock_mouse() const;
		void unlock_mouse() const;

		void show_mouse() const;
		void hide_mouse() const;

	private:
#ifdef VKB_WINDOWS
		static uint16_t class_id;

		static LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);
#elif defined(VKB_LINUX)
		static wayland wl_;

		static void libdecor_configure(libdecor_frame*         frame,
		                               libdecor_configuration* configuration, void* ud);
		static void libdecor_close(libdecor_frame* frame, void* ud);
		static void libdecor_commit(libdecor_frame* frame, void* ud);
		static void libdecor_dismiss_popup(struct libdecor_frame* frame,
		                                   char const* seat_name, void* ud);

		wl_surface*     surface_ {nullptr};
		libdecor_frame* frame_;
		uint32_t        state_;

		uint32_t w_ {0};
		uint32_t h_ {0};
#endif

		[[maybe_unused]] void*         handle_;
		[[maybe_unused]] input_system* is_;

		[[maybe_unused]] bool closed_ {false};
		[[maybe_unused]] bool min_ {false};
	};
}