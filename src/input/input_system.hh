#pragma once

#include <stdint.h>

#include <pair.hh>

#include "keys.hh"

namespace vkb
{
	class window;

	class input_system
	{
#ifdef VKB_WINDOWS
		friend window;
#endif

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
#ifdef VKB_WINDOWS
		void handle_event(uint64_t w_param, uint64_t l_param);

		uint8_t*                   key_states_;
		mc::pair<float, float>     wheel_ {0.f, 0.f};
		mc::pair<int32_t, int32_t> pos_abs_ {0, 0};
		mc::pair<int32_t, int32_t> pos_rel_ {0, 0};
#endif
	};
};