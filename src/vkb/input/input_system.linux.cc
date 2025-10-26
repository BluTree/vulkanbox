#include "input_system.hh"

namespace vkb
{
	input_system::input_system() {}

	input_system::~input_system() {}

	void input_system::clear_transitions() {}

	bool input_system::pressed([[maybe_unused]] key k) const
	{
		return false;
	}

	bool input_system::released([[maybe_unused]] key k) const
	{
		return false;
	}

	bool input_system::just_pressed([[maybe_unused]] key k) const
	{
		return false;
	}

	bool input_system::just_released([[maybe_unused]] key k) const
	{
		return false;
	}

	mc::pair<float, float> input_system::mouse_wheel() const
	{
		return {0.f, 0.f};
	}

	mc::pair<int32_t, int32_t> input_system::mouse_pos() const
	{
		return {0, 0};
	}

	mc::pair<int32_t, int32_t> input_system::mouse_delta() const
	{
		return {0, 0};
	}
}