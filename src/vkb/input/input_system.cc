#include "input_system.hh"

namespace vkb
{
	void input_system::clear_transitions()
	{
		for (mc::underlying_type<key> i {0}; i < mc::to_underlying(key::max_enum); ++i)
			key_states_[i * 2 / 8] &= ~(1 << (i * 2 % 8 + 1));

		wheel_ = {0.f, 0.f};
		pos_rel_ = {0, 0};
	}

	bool input_system::pressed(key k) const
	{
		return (key_states_[mc::to_underlying(k) * 2 / 8] &
		        (1 << (mc::to_underlying(k) * 2 % 8))) != 0;
	}

	bool input_system::released(key k) const
	{
		return (key_states_[mc::to_underlying(k) * 2 / 8] &
		        (1 << (mc::to_underlying(k) * 2 % 8))) == 0;
	}

	bool input_system::just_pressed(key k) const
	{
		return ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << (mc::to_underlying(k) * 2 % 8))) != 0) &&
		       ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << ((mc::to_underlying(k) * 2 + 1) % 8))) != 0);
	}

	bool input_system::just_released(key k) const
	{
		return ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << (mc::to_underlying(k) * 2 % 8))) == 0) &&
		       ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << ((mc::to_underlying(k) * 2 + 1) % 8))) != 0);
	}

	mc::pair<float, float> input_system::mouse_wheel() const
	{
		return wheel_;
	}

	mc::pair<int32_t, int32_t> input_system::mouse_pos() const
	{
		return pos_abs_;
	}

	mc::pair<int32_t, int32_t> input_system::mouse_delta() const
	{
		return pos_rel_;
	}

	void input_system::set_state(key k, bool pressed)
	{
		bool last_pressed = (key_states_[mc::to_underlying(k) * 2 / 8] &
		                     (1 << (mc::to_underlying(k) * 2 % 8))) != 0;

		if (pressed)
		{
			key_states_[mc::to_underlying(k) * 2 / 8] |=
				(1 << (mc::to_underlying(k) * 2 % 8));

			if (!last_pressed)
				key_states_[mc::to_underlying(k) * 2 / 8] |=
					(1 << (mc::to_underlying(k) * 2 % 8 + 1));
		}
		else
		{
			key_states_[mc::to_underlying(k) * 2 / 8] &=
				~(1 << (mc::to_underlying(k) * 2 % 8));

			if (last_pressed)
				key_states_[mc::to_underlying(k) * 2 / 8] |=
					(1 << (mc::to_underlying(k) * 2 % 8 + 1));
		}
	}
}