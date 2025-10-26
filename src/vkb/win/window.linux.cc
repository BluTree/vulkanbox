#include "window.hh"

#include "../log.hh"

#include <string.h>

#include <libdecor.h>
#include <wayland-xdg-decoration-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>

namespace vkb
{
	namespace
	{
		void seat_capabilities(void* ud, wl_seat* seat, uint32_t capabilities)
		{
			(void)ud;
			(void)seat;
			(void)capabilities;
		}

		void seat_name(void* ud, [[maybe_unused]] wl_seat* seat, char const* name)
		{
			window::wayland* wl = reinterpret_cast<window::wayland*>(ud);
			wl->seat_name_ = name;
		}

		void wm_base_ping([[maybe_unused]] void* ud, xdg_wm_base* wm_base,
		                  uint32_t serial)
		{
			xdg_wm_base_pong(wm_base, serial);
		}

		void registry_handle(void* ud, wl_registry* registry, uint32_t id,
		                     char const* interface, uint32_t version)
		{
			window::wayland* wl = reinterpret_cast<window::wayland*>(ud);
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

					static wl_seat_listener const listener = {seat_capabilities,
					                                          seat_name};
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
				wl->decoration_mgr =
					reinterpret_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(
						registry, id, &zxdg_decoration_manager_v1_interface, 1));
			}
		}

		void registry_remove(void* ud, wl_registry* registry, uint32_t id)
		{
			[[maybe_unused]] window::wayland* wl = reinterpret_cast<window::wayland*>(ud);

			(void)registry;
			(void)id;
		}

		void libdecor_error_handle([[maybe_unused]] libdecor*      context,
		                           [[maybe_unused]] libdecor_error error,
		                           char const*                     message)
		{
			log::error("libdecor: %s", message);
		}

		void init_wl(window::wayland& wl)
		{
			wl.display_ = wl_display_connect(nullptr);
			log::assert(wl.display_ != nullptr, "Unable to connect Wayland display");

			wl_registry* reg = wl_display_get_registry(wl.display_);
			log::assert(reg != nullptr, "Unable to get Wayland registry");

			static wl_registry_listener const reg_listener = {registry_handle,
			                                                  registry_remove};

			wl_registry_add_listener(reg, &reg_listener, &wl);
			wl_display_dispatch_pending(wl.display_);
			wl_display_roundtrip(wl.display_);
			wl_display_roundtrip(wl.display_);
			wl_display_dispatch_pending(wl.display_);

			wl_display_sync(wl.display_);

			if (!wl.decoration_mgr)
			{
				static libdecor_interface libdecor_interface {};
				libdecor_interface.error = libdecor_error_handle;
				wl.libdecor_ = libdecor_new(wl.display_, &libdecor_interface);
			}
		}
	}

	window::wayland window::wl_ {};

	window::window(input_system* is)
	: is_ {is}
	{
		if (!wl_.display_)
			init_wl(wl_);

		surface_ = wl_compositor_create_surface(wl_.compositor_);

		if (wl_.libdecor_)
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

			frame_ = libdecor_decorate(wl_.libdecor_, surface_, &libdecor_frame_interface,
			                           this);
			libdecor_frame_set_title(frame_, "main_window");
			libdecor_frame_set_app_id(frame_, "vulkanbox");
			libdecor_frame_map(frame_);
		}
		else
		{
			// TODO Server side decoration
		}

		wl_surface_commit(surface_);
		wl_display_roundtrip(wl_.display_);
		wl_surface_commit(surface_);
	}

	window::window(window&& other)
	{
		// TODO
		(void)other;
	}

	window::~window() {}

	window& window::operator=(window&& other)
	{
		// TODO
		(void)other;

		return *this;
	}

	void window::update()
	{
		libdecor_dispatch(wl_.libdecor_, 0);
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
		return surface_;
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

	window::wayland const& window::wayland_context()
	{
		return wl_;
	}

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

		wl_surface_commit(win->surface_);

		win->w_ = w;
		win->h_ = h;

		uint32_t changed_state = win->state_ ^ window_state;

		if ((changed_state & LIBDECOR_WINDOW_STATE_ACTIVE) != 0 ||
		    (changed_state & LIBDECOR_WINDOW_STATE_SUSPENDED) != 0)
		{
			log::info("window: focus/unfocus");
		}
		else
		{
			asm volatile("int3;");
		}

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
		wl_surface_commit(win->surface_);
	}

	void window::libdecor_dismiss_popup([[maybe_unused]] struct libdecor_frame* frame,
	                                    [[maybe_unused]] char const*            seat_name,
	                                    [[maybe_unused]] void*                  ud)
	{
		// popup_destroy(window->popup);
	}
} // namespace vkb
