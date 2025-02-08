#pragma once

#include <stdint.h>

#ifdef VKB_WINDOWS
#include <win32/window.h>
#endif

namespace vkb
{
	class window
	{
	public:
		window();
		window(window const&) = delete;
		window(window&&);
		~window();

		window& operator=(window const&) = delete;
		window& operator=(window&&);

		void update();

		bool  closed() const;
		void* native_handle() const;

	private:
#ifdef VKB_WINDOWS
		static uint16_t class_id;

		static LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);
#endif

		void* handle_;

		bool closed_ {false};
	};
}