#include "orbital.hh"

#include "../input/input_system.hh"
#include "../win/window.hh"

#include "../math/quat.hh"
#include "../math/trig.hh"
#include <math.h>

namespace vkb::cam
{
	orbital::orbital(input_system& is, window& win)
	: is_ {is}
	, win_ {win}
	{}

	void orbital::update([[maybe_unused]] double dt)
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

		quat rot = quat::angle_axis({1.f, 0.f, 0.f, 0.f}, rad(-pitch_)) *
		           quat::angle_axis({0.f, 0.f, 1.f, 0.f}, rad(-yaw_));

		auto [scroll_x, scroll_y] = is_.mouse_wheel();
		zoom_ -= scroll_y * 0.1;
		if (zoom_ > 2.f)
			zoom_ = 2.f;
		if (zoom_ < 0.3f)
			zoom_ = 0.3f;

		vec4 vel {0.f, 0.f, 0.f, 1.f};
		if (is_.pressed(key::w))
			vel.y += dt * 5;
		if (is_.pressed(key::s))
			vel.y -= dt * 5;

		if (is_.pressed(key::a))
			vel.x -= dt * 5;
		if (is_.pressed(key::d))
			vel.x += dt * 5;

		if (is_.pressed(key::l_shift))
			vel.z += dt * 5;
		if (is_.pressed(key::l_ctrl))
			vel.z -= dt * 5;

		vel.norm3();
		vel = quat::angle_axis({0.f, 0.f, 1.f, 1.f}, rad(-yaw_)).rotate(vel);
		view_pos_ += vel;

		vec4 view_axis {0.f, zoom_ * -10.f, 0.f, 0.f};
		view_axis = rot.rotate(view_axis);
		vec4 cam_pos = view_pos_ + view_axis;

		// +90 to pitch, to turn the z axis to up axis, and y axis to front axis
		rot_mat_ = mat4::rotate({0.f, 0.f, 1.f, 0.f}, rad(-yaw_)) *
		           mat4::rotate({1.f, 0.f, 0.f, 0.f}, rad(-pitch_ + 90));
		view_mat_ = mat4::translate(-cam_pos) * rot_mat_;
	}

	vec4 orbital::view_pos() const
	{
		return view_pos_;
	}
}