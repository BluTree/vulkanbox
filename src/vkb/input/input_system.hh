#pragma once

#include <stdint.h>

#include <enum.hh>
#include <pair.hh>

#include "keys.hh"

namespace vkb
{
	class window;

	class input_system
	{
		friend window;

	public:
		input_system();
		input_system(input_system const&) = delete;
		input_system(input_system&&) = delete;
		~input_system();

		input_system& operator=(input_system const&) = delete;
		input_system& operator=(input_system&&) = delete;

		void clear_transitions();

		bool pressed(key k) const;
		bool released(key k) const;
		bool just_pressed(key k) const;
		bool just_released(key k) const;

		mc::pair<float, float>     mouse_wheel() const;
		mc::pair<int32_t, int32_t> mouse_pos() const;
		mc::pair<int32_t, int32_t> mouse_delta() const;

	private:
		void set_state(key k, bool pressed);

#ifdef VKB_WINDOWS
		void handle_event(uint64_t w_param, uint64_t l_param);
#elif defined(VKB_LINUX)
		void pointer_motion(int32_t x, int32_t y);
		void pointer_button(uint32_t button, uint32_t state);
		void pointer_axis(uint32_t axis, int32_t state);

		void keyboard_key(uint32_t key, uint32_t state);
#endif
		uint8_t key_states_[(mc::to_underlying(key::max_enum) * 2) / 8 + 1];
		mc::pair<float, float>     wheel_ {0.f, 0.f};
		mc::pair<int32_t, int32_t> pos_abs_ {0, 0};
		mc::pair<int32_t, int32_t> pos_rel_ {0, 0};
	};
};