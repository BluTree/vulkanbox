#include "free.hh"

#include "../input/input_system.hh"
#include "../win/window.hh"

#include "../math/trig.hh"
#include <math.h>

namespace vkb::cam
{
	free::free(input_system& is, window& win)
	: is_ {is}
	, win_ {win}
	{}

	void free::update(double dt)
	{
		if (is_.just_pressed(key::m2))
		{
			win_.lock_mouse();
			win_.hide_mouse();
		}
		else if (is_.just_released(key::m2))
		{
			win_.unlock_mouse();
			win_.show_mouse();
		}

		if (is_.pressed(key::m2))
		{
			auto [delta_x, delta_y] = is_.mouse_delta();
			yaw_ = fmodf(yaw_ + delta_x * .2, 360.f);

			pitch_ += delta_y * .2;
		}

		rot_ = quat::angle_axis({1.f, 0.f, 0.f, 1.f}, rad(-pitch_)) *
		       quat::angle_axis({0.f, 0.f, 1.f, 1.f}, rad(-yaw_));

		vec4 vel {0.f, 0.f, 0.f, 1.f};
		if (is_.pressed(key::w))
			vel.y += dt * 5;
		if (is_.pressed(key::s))
			vel.y -= dt * 5;

		if (is_.pressed(key::a))
			vel.x -= dt * 5;
		if (is_.pressed(key::d))
			vel.x += dt * 5;

		float vel_modifier = 1.f;
		if (is_.pressed(key::l_shift))
			vel_modifier = 2.f;
		if (is_.pressed(key::l_ctrl))
			vel_modifier = 0.5f;

		vel.norm3();
		pos_ += rot_.rotate(vel * vel_modifier);
	}

	mat4 free::view_mat()
	{
		// +90 to pitch, to turn the z axis to up axis, and y axis to front axis
		return mat4::translate(-pos_) * mat4::rotate({0.f, 0.f, 1.f, 1.f}, rad(-yaw_)) *
		       mat4::rotate({1.f, 0.f, 0.f, 1.f}, rad(-pitch_ + 90));
	}
}