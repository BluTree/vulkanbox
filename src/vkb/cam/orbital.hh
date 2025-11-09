#pragma once

#include "../math/mat4.hh"

#include "../math/vec4.hh"

#include "base.hh"

namespace vkb
{
	class input_system;
	class window;

	namespace ui
	{
		class context;
	}
}

namespace vkb::cam
{
	class orbital : public base
	{
		friend ui::context;

	public:
		orbital(input_system& is, window& win);

		void update(double dt);

		vec4 view_pos() const;

	private:
		input_system& is_;
		window&       win_;

		vec4  view_pos_ {0.f, 0.f, 0.f, 1.f};
		float yaw_ {0.f};
		float pitch_ {0.f};
		float zoom_ {1.f};
	};
}