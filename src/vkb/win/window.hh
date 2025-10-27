#pragma once

#include <stdint.h>

#ifdef VKB_WINDOWS
#include <win32/window.h>
#endif

#include <pair.hh>
#include <string_view.hh>

namespace vkb
{
	class input_system;
	class display;
}
#ifdef VKB_LINUX
struct libdecor_frame;
struct libdecor_configuration;
#endif

namespace vkb
{
	class window
	{
		friend display;

	public:
		window(mc::string_view name, input_system* is = nullptr);
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
		static void libdecor_configure(libdecor_frame*         frame,
		                               libdecor_configuration* configuration, void* ud);
		static void libdecor_close(libdecor_frame* frame, void* ud);
		static void libdecor_commit(libdecor_frame* frame, void* ud);
		static void libdecor_dismiss_popup(struct libdecor_frame* frame,
		                                   char const* seat_name, void* ud);

		libdecor_frame* frame_;
		uint32_t        state_;

		uint32_t w_ {0};
		uint32_t h_ {0};
#endif

		void*         handle_;
		input_system* is_;

		bool closed_ {false};
		bool min_ {false};
	};
}