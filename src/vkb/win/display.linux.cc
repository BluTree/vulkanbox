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
		display* wl = reinterpret_cast<display*>(ud);
		if (strcmp(interface, wl_compositor_interface.name) == 0)
		{
			if (version >= 4)
			{
				wl->compositor_ = reinterpret_cast<wl_compositor*>(
					wl_registry_bind(registry, id, &wl_compositor_interface, 4));
			}
		}
		else if (strcmp(interface, wl_shm_interface.name) == 0)
		{
			wl->shm_ = reinterpret_cast<wl_shm*>(
				wl_registry_bind(registry, id, &wl_shm_interface, 1));
		}
		else if (strcmp(interface, wl_seat_interface.name) == 0)
		{
			if (version >= 3)
			{
				wl->seat_ = reinterpret_cast<wl_seat*>(
					wl_registry_bind(registry, id, &wl_seat_interface, 3));

				static wl_seat_listener const listener = {seat_capabilities, seat_name};
				wl_seat_add_listener(wl->seat_, &listener, wl);
			}
		}
		else if (strcmp(interface, wl_output_interface.name) == 0)
		{
			if (version >= 2)
			{
				wl->output_ = reinterpret_cast<wl_output*>(
					wl_registry_bind(registry, id, &wl_output_interface, 3));

				// TODO output
			}
		}
		else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
		{
			wl->wm_base_ = reinterpret_cast<xdg_wm_base*>(
				wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));

			static xdg_wm_base_listener const wm_base_listener = {wm_base_ping};

			xdg_wm_base_add_listener(wl->wm_base_, &wm_base_listener, ud);
		}
		else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
		{
			wl->decoration_mgr = reinterpret_cast<zxdg_decoration_manager_v1*>(
				wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
		}
	}

	void display::registry_remove(void* ud, wl_registry* registry, uint32_t id)
	{
		(void)ud;
		(void)registry;
		(void)id;
	}

	void display::seat_capabilities(void* ud, wl_seat* seat, uint32_t capabilities)
	{
		(void)ud;
		(void)seat;
		(void)capabilities;
	}

	void display::seat_name(void* ud, wl_seat* seat, char const* name)
	{
		(void)ud;
		(void)seat;
		(void)name;
	}

	void display::wm_base_ping([[maybe_unused]] void* ud, xdg_wm_base* wm_base,
	                           uint32_t serial)
	{
		xdg_wm_base_pong(wm_base, serial);
	}
}