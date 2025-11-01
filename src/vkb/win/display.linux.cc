#include "display.hh"

#include "../log.hh"
#include "window.hh"

#include <libdecor.h>
#include <wayland-client.h>
#include <wayland-xdg-decoration-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>

#include <string.h>

namespace vkb
{
	namespace
	{
		void libdecor_error_handle([[maybe_unused]] libdecor*      context,
		                           [[maybe_unused]] libdecor_error error,
		                           char const*                     message)
		{
			log::error("libdecor: %s", message);
		}
	}

	display* display::instance_ {nullptr};

	display& display::get()
	{
		return *instance_;
	}

	display::display()
	{
		instance_ = this;

		display_ = wl_display_connect(nullptr);
		log::assert(display_ != nullptr, "Unable to connect Wayland display");

		wl_registry* reg = wl_display_get_registry(display_);
		log::assert(reg != nullptr, "Unable to get Wayland registry");

		static wl_registry_listener const reg_listener = {registry_handle,
		                                                  registry_remove};

		wl_registry_add_listener(reg, &reg_listener, this);
		wl_display_dispatch_pending(display_);
		wl_display_roundtrip(display_);
		wl_display_roundtrip(display_);
		wl_display_dispatch_pending(display_);

		wl_display_sync(display_);

		if (!decoration_mgr)
		{
			static libdecor_interface libdecor_interface {};
			libdecor_interface.error = libdecor_error_handle;
			libdecor_ = libdecor_new(display_, &libdecor_interface);
		}
		else
		{
			// TODO Server side
		}
	}

	display::~display() {}

	void display::update()
	{
		if (libdecor_)
			libdecor_dispatch(libdecor_, 0);
		// TODO Server side
	}

	wl_display* display::get_display()
	{
		return display_;
	}

	wl_compositor* display::get_compositor()
	{
		return compositor_;
	}

	libdecor* display::get_libdecor()
	{
		return libdecor_;
	}

	void display::add_window(window* win)
	{
		surface_to_win_.emplace_back(reinterpret_cast<wl_surface*>(win->handle_), win);
	}

	void display::remove_window(window* win)
	{
		for (uint32_t i {0}; i < surface_to_win_.size(); ++i)
		{
			if (surface_to_win_[i].second == win)
			{
				surface_to_win_.swap_pop_back(i);
				break;
			}
		}
	}

	void display::registry_handle(void* ud, wl_registry* registry, uint32_t id,
	                              char const* interface, uint32_t version)
	{
		display* disp = reinterpret_cast<display*>(ud);
		if (strcmp(interface, wl_compositor_interface.name) == 0)
		{
			if (version >= 4)
			{
				disp->compositor_ = reinterpret_cast<wl_compositor*>(
					wl_registry_bind(registry, id, &wl_compositor_interface, 4));
			}
		}
		else if (strcmp(interface, wl_shm_interface.name) == 0)
		{
			disp->shm_ = reinterpret_cast<wl_shm*>(
				wl_registry_bind(registry, id, &wl_shm_interface, 1));
		}
		else if (strcmp(interface, wl_seat_interface.name) == 0)
		{
			if (version >= 3)
			{
				disp->seat_ = reinterpret_cast<wl_seat*>(
					wl_registry_bind(registry, id, &wl_seat_interface, 3));

				static wl_seat_listener const listener = {seat_capabilities, seat_name};
				wl_seat_add_listener(disp->seat_, &listener, disp);
			}
		}
		else if (strcmp(interface, wl_output_interface.name) == 0)
		{
			if (version >= 2)
			{
				disp->output_ = reinterpret_cast<wl_output*>(
					wl_registry_bind(registry, id, &wl_output_interface, 3));

				// TODO output
			}
		}
		else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
		{
			disp->wm_base_ = reinterpret_cast<xdg_wm_base*>(
				wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));

			static xdg_wm_base_listener const wm_base_listener = {wm_base_ping};

			xdg_wm_base_add_listener(disp->wm_base_, &wm_base_listener, ud);
		}
		else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
		{
			disp->decoration_mgr = reinterpret_cast<zxdg_decoration_manager_v1*>(
				wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
		}
	}

	void display::registry_remove(void* ud, wl_registry* registry, uint32_t id)
	{
		(void)ud;
		(void)registry;
		(void)id;
	}

	void display::wm_base_ping([[maybe_unused]] void* ud, xdg_wm_base* wm_base,
	                           uint32_t serial)
	{
		xdg_wm_base_pong(wm_base, serial);
	}

	void display::seat_name(void* ud, wl_seat* seat, char const* name)
	{
		(void)ud;
		(void)seat;
		(void)name;
	}

	void display::seat_capabilities(void* ud, wl_seat* seat, uint32_t capabilities)
	{
		display* disp = reinterpret_cast<display*>(ud);

		if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !disp->pointer_)
		{
			disp->pointer_ = wl_seat_get_pointer(seat);

			static wl_pointer_listener const pointer_listener {
				display::pointer_enter,
				display::pointer_leave,
				display::pointer_motion,
				display::pointer_button,
				display::pointer_axis,
				nullptr /*ponter_frame*/,
				nullptr /*pointer_axis_source*/,
				nullptr /*pointer_axis_stop*/,
				nullptr /*pointer_axis_discrete*/,
				nullptr /*pointer_axis_value120*/,
				nullptr /*pointer_axis_relative_direction*/,
			};
			wl_pointer_add_listener(disp->pointer_, &pointer_listener, disp);
		}
		else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && disp->pointer_)
		{
			wl_pointer_release(disp->pointer_);
			disp->pointer_ = nullptr;
		}

		if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
		{
			disp->keyboard_ = wl_seat_get_keyboard(seat);

			static wl_keyboard_listener const keyboard_listener {
				display::keyboard_keymap,    display::keyboard_enter,
				display::keyboard_leave,     display::keyboard_key,
				display::keyboard_modifiers, display::keyboard_repeat_info,
			};
			wl_keyboard_add_listener(disp->keyboard_, &keyboard_listener, disp);
		}
	}

	void display::pointer_enter([[maybe_unused]] void*       ud,
	                            [[maybe_unused]] wl_pointer* wl_pointer,
	                            [[maybe_unused]] uint32_t    serial,
	                            [[maybe_unused]] wl_surface* surface,
	                            [[maybe_unused]] int32_t     surface_x,
	                            [[maybe_unused]] int32_t     surface_y)
	{
		display* disp = reinterpret_cast<display*>(ud);

		for (uint32_t i {0}; i < disp->surface_to_win_.size(); ++i)
		{
			if (disp->surface_to_win_[i].first == surface)
			{
				disp->pointer_window_ = disp->surface_to_win_[i].second;
				break;
			}
		}
	}

	void display::pointer_leave([[maybe_unused]] void*       ud,
	                            [[maybe_unused]] wl_pointer* pointer,
	                            [[maybe_unused]] uint32_t    serial,
	                            [[maybe_unused]] wl_surface* surface)
	{
		display* disp = reinterpret_cast<display*>(ud);

		disp->pointer_window_ = nullptr;
	}

	void display::pointer_motion(void* ud, [[maybe_unused]] wl_pointer* pointer,
	                             [[maybe_unused]] uint32_t time, int32_t surface_x,
	                             int32_t surface_y)
	{
		display* disp = reinterpret_cast<display*>(ud);

		for (uint32_t i {0}; i < disp->surface_to_win_.size(); ++i)
			disp->surface_to_win_[i].second->pointer_motion(surface_x, surface_y);
	}

	void display::pointer_button(void* ud, [[maybe_unused]] wl_pointer* pointer,
	                             [[maybe_unused]] uint32_t serial,
	                             [[maybe_unused]] uint32_t time, uint32_t button,
	                             uint32_t state)
	{
		display* disp = reinterpret_cast<display*>(ud);

		if (!disp->pointer_window_)
			return;

		disp->pointer_window_->pointer_button(button, state);
	}

	void display::pointer_axis(void* ud, [[maybe_unused]] wl_pointer* pointer,
	                           [[maybe_unused]] uint32_t time, uint32_t axis,
	                           int32_t value)
	{
		display* disp = reinterpret_cast<display*>(ud);

		// TODO always process pointer axis, to be constant with windows ?
		if (!disp->pointer_window_)
			return;

		disp->pointer_window_->pointer_axis(axis, value);
	}

	void display::keyboard_keymap([[maybe_unused]] void*        ud,
	                              [[maybe_unused]] wl_keyboard* keyboard,
	                              [[maybe_unused]] uint32_t     format,
	                              [[maybe_unused]] int32_t      fd,
	                              [[maybe_unused]] uint32_t     size)
	{}

	void display::keyboard_enter(void* ud, [[maybe_unused]] wl_keyboard* keyboard,
	                             [[maybe_unused]] uint32_t serial, wl_surface* surface,
	                             [[maybe_unused]] wl_array* keys)
	{
		display* disp = reinterpret_cast<display*>(ud);

		for (uint32_t i {0}; i < disp->surface_to_win_.size(); ++i)
		{
			if (disp->surface_to_win_[i].first == surface)
			{
				disp->keyboard_window_ = disp->surface_to_win_[i].second;
				break;
			}
		}
	}

	void display::keyboard_leave(void* ud, [[maybe_unused]] wl_keyboard* keyboard,
	                             [[maybe_unused]] uint32_t    serial,
	                             [[maybe_unused]] wl_surface* surface)
	{
		display* disp = reinterpret_cast<display*>(ud);

		disp->keyboard_window_ = nullptr;
	}

	void display::keyboard_key(void* ud, [[maybe_unused]] wl_keyboard* keyboard,
	                           [[maybe_unused]] uint32_t serial,
	                           [[maybe_unused]] uint32_t time, uint32_t key,
	                           uint32_t state)
	{
		display* disp = reinterpret_cast<display*>(ud);

		if (!disp->keyboard_window_)
			return;

		disp->keyboard_window_->keyboard_key(key, state);
	}

	void display::keyboard_modifiers([[maybe_unused]] void*        ud,
	                                 [[maybe_unused]] wl_keyboard* keyboard,
	                                 [[maybe_unused]] uint32_t     serial,
	                                 [[maybe_unused]] uint32_t     mods_depressed,
	                                 [[maybe_unused]] uint32_t     mods_latched,
	                                 [[maybe_unused]] uint32_t     mods_locked,
	                                 [[maybe_unused]] uint32_t     group)
	{}

	void display::keyboard_repeat_info([[maybe_unused]] void*        ud,
	                                   [[maybe_unused]] wl_keyboard* keyboard,
	                                   [[maybe_unused]] int32_t      rate,
	                                   [[maybe_unused]] int32_t      delay)
	{}
} // namespace vkb