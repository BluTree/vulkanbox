#include "window.hh"

#include "../input/input_system.hh"
#include "log.hh"

#include <imgui/backends/imgui_impl_win32.h>
#include <win32/misc.h>

#include <string.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(struct HWND__* hWnd,
                                                             UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

namespace vkb
{
	uint16_t window::class_id {0};

	window::window(input_system* is)
	{
		is_ = is;
		HMODULE instance = GetModuleHandleW(nullptr);
		if (!window::class_id)
		{
			WNDCLASSEXW wnd_class;
			memset(&wnd_class, 0, sizeof(WNDCLASSEXW));
			wnd_class.cbSize = sizeof(WNDCLASSEXW);
			wnd_class.lpfnWndProc = wnd_proc;
			wnd_class.hInstance = instance;
			wnd_class.hIcon = nullptr;
			wnd_class.hbrBackground = (HBRUSH)(1 /*COLOR_BACKGROUND*/ + 1);
			wnd_class.lpszClassName = L"vulkanbox_window";

			window::class_id = RegisterClassExW(&wnd_class);
		}

		handle_ = CreateWindowExW(0, L"vulkanbox_window", L"main_window",
		                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000,
		                          800, nullptr, nullptr, instance, nullptr);
		if (!handle_)
		{
			log::error("Cannot create window");
			closed_ = true;
		}
		else
		{
			SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

			ShowWindow(handle_, SW_NORMAL);
		}
	}

	window::window([[maybe_unused]] window&& other)
	: handle_ {other.handle_}
	{
		SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		other.handle_ = nullptr;
	}

	window::~window()
	{
		if (handle_)
			DestroyWindow(handle_);
	}

	window& window::operator=([[maybe_unused]] window&& other)
	{
		handle_ = other.handle_;
		SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		other.handle_ = nullptr;
		return *this;
	}

	void window::update()
	{
		if (!handle_)
			return;

		if (closed_)
		{
			handle_ = nullptr;
			return;
		}

		MSG msg;
		while (PeekMessageW(&msg, handle_, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	bool window::closed() const
	{
		return closed_;
	}

	bool window::minimized() const
	{
		return min_;
	}

	void* window::native_handle() const
	{
		return handle_;
	}

	mc::pair<uint32_t, uint32_t> window::size() const
	{
		RECT rect;
		GetClientRect(handle_, &rect);
		return {static_cast<uint32_t>(rect.right), static_cast<uint32_t>(rect.bottom)};
	}

	mc::pair<int32_t, int32_t> window::position() const
	{
		RECT rect;
		GetWindowRect(handle_, &rect);
		return {rect.left, rect.top};
	}

	void window::lock_mouse() const
	{
		RECT rect;
		GetWindowRect(handle_, &rect);

		rect.top = rect.bottom = rect.top + (rect.bottom - rect.top) / 2;
		rect.left = rect.right = rect.left + (rect.right - rect.left) / 2;
		ClipCursor(&rect);
	}

	void window::unlock_mouse() const
	{
		ClipCursor(nullptr);
	}

	void window::show_mouse() const
	{
		ShowCursor(true);
	}

	void window::hide_mouse() const
	{
		ShowCursor(false);
	}

	LRESULT window::wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		window* win = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		// Should never happen, since the only window class registering this wnd_proc is
		// ours.
		if (!win)
			return DefWindowProcW(hwnd, msg, w_param, l_param);

		if (ImGui_ImplWin32_WndProcHandler((struct HWND__*)hwnd, msg, w_param, l_param))
			return true;

		switch (msg)
		{
			case WM_CLOSE:
			case WM_QUIT: DestroyWindow(win->handle_); break;
			case WM_DESTROY:
				win->closed_ = true;
				win->handle_ = nullptr;
				break;

			case WM_SIZE:
				if (w_param == 1 /*SIZE_MINIMIZED*/)
					win->min_ = true;
				else
					win->min_ = false;
				break;

			case 0x00FF: // TODO WM_INPUT
				if (win->is_)
					win->is_->handle_event(w_param, l_param);
				break;

			default: break;
		}
		return DefWindowProcW(hwnd, msg, w_param, l_param);
	}
}