#include "window.hh"

#include "../log.hh"
#include "display.hh"

#include <string.h>

#include <libdecor.h>

namespace vkb
{
	window::window(mc::string_view name, input_system* is)
	: is_ {is}
	{
		display& disp {display::get()};

		handle_ =
			reinterpret_cast<void*>(wl_compositor_create_surface(disp.get_compositor()));

		if (disp.get_libdecor())
		{
			static struct libdecor_frame_interface libdecor_frame_interface {
				libdecor_configure,
				libdecor_close,
				libdecor_commit,
				libdecor_dismiss_popup,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
			};

			frame_ = libdecor_decorate(disp.get_libdecor(),
			                           reinterpret_cast<wl_surface*>(handle_),
			                           &libdecor_frame_interface, this);
			libdecor_frame_set_title(frame_, name.data());
			libdecor_frame_set_app_id(frame_, "vulkanbox");
			libdecor_frame_map(frame_);
		}
		else
		{
			// TODO Server side decoration
		}

		wl_surface_commit(reinterpret_cast<wl_surface*>(handle_));
		wl_display_roundtrip(disp.get_display());
		wl_surface_commit(reinterpret_cast<wl_surface*>(handle_));

		disp.add_window(this);
	}

	window::~window() {}

	void window::update() {}

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
		return {w_, h_};
	}

	mc::pair<int32_t, int32_t> window::position() const
	{
		return {0, 0};
	}

	void window::lock_mouse() const {}

	void window::unlock_mouse() const {}

	void window::show_mouse() const {}

	void window::hide_mouse() const {}

	void window::libdecor_configure(libdecor_frame*         frame,
	                                libdecor_configuration* configuration, void* ud)
	{
		window* win = reinterpret_cast<window*>(ud);

		int32_t               w {0}, h {0};
		libdecor_window_state window_state;
		libdecor_state*       state;

		if (libdecor_configuration_get_window_state(configuration, &window_state) ==
		    false)
		{
			window_state = LIBDECOR_WINDOW_STATE_NONE;
		}

		// libdecor_frame_unset_fullscreen(frame);

		if (libdecor_configuration_get_content_size(configuration, frame, &w, &h) ==
		    false)
		{
			w = 1000;
			h = 800;
		}

		state = libdecor_state_new(w, h);
		libdecor_frame_commit(frame, state, configuration);
		libdecor_state_free(state);

		wl_surface_commit(reinterpret_cast<wl_surface*>(win->handle_));

		win->w_ = w;
		win->h_ = h;

		// uint32_t changed_state = win->state_ ^ window_state;

		// if ((changed_state & LIBDECOR_WINDOW_STATE_ACTIVE) != 0 ||
		//     (changed_state & LIBDECOR_WINDOW_STATE_SUSPENDED) != 0)
		// {
		// 	log::info("window: focus/unfocus");
		// }
		// else
		// {
		// 	asm volatile("int3;");
		// }

		win->state_ = window_state;
	}

	void window::libdecor_close([[maybe_unused]] libdecor_frame* frame, void* ud)
	{
		window* win = reinterpret_cast<window*>(ud);
		win->closed_ = true;
	}

	void window::libdecor_commit([[maybe_unused]] libdecor_frame* frame, void* ud)
	{
		window* win = reinterpret_cast<window*>(ud);
		wl_surface_commit(reinterpret_cast<wl_surface*>(win->handle_));
	}

	void window::libdecor_dismiss_popup([[maybe_unused]] struct libdecor_frame* frame,
	                                    [[maybe_unused]] char const*            seat_name,
	                                    [[maybe_unused]] void*                  ud)
	{
		// popup_destroy(window->popup);
	}
} // namespace vkb
