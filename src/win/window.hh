#pragma once

#include <stdint.h>

#ifdef VKB_WINDOWS
#include <win32/window.h>
#endif

#include <pair.hh>

namespace vkb
{
	class input_system;
}

namespace vkb
{
	class window
	{
	public:
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
#endif

		void*         handle_;
		input_system* is_;

		bool closed_ {false};
		bool min_ {false};
	};
}