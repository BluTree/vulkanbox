#include "display.hh"

#include "window.hh"

#include <string.h>
#include <win32/misc.h>
#include <win32/window.h>

namespace vkb
{

	display::display()
	{
		instance_ = this;

		HMODULE instance = GetModuleHandleW(nullptr);

		WNDCLASSEXW wnd_class;
		memset(&wnd_class, 0, sizeof(WNDCLASSEXW));
		wnd_class.cbSize = sizeof(WNDCLASSEXW);
		wnd_class.lpfnWndProc = window::wnd_proc;
		wnd_class.hInstance = instance;
		wnd_class.hIcon = nullptr;
		wnd_class.hbrBackground = (HBRUSH)(1 /*COLOR_BACKGROUND*/ + 1);
		wnd_class.lpszClassName = L"vulkanbox_window";

		class_id_ = RegisterClassExW(&wnd_class);
	}

	display::~display() {}

	void display::update()
	{
		for (uint32_t i {0}; i < windows_.size(); ++i)
			windows_[i]->update();
	}

	void display::add_window(window* win)
	{
		windows_.emplace_back(win);
	}

	void display::remove_window(window* win)
	{
		for (uint32_t i {0}; i < windows_.size(); ++i)
		{
			if (windows_[i] == win)
			{
				windows_.swap_pop_back(i);
				break;
			}
		}
	}
}