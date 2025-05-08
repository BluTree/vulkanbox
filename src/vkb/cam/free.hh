#pragma once

#include "../math/mat4.hh"
#include "../math/quat.hh"
#include "../math/vec4.hh"

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
	class free
	{
		friend ui::context;

	public:
		free(input_system& is, window& win);

		void update(double dt);

		mat4 view_mat();

	private:
		input_system& is_;
		window&       win_;

		vec4  pos_ {0.f, 0.f, 0.f, 1.f};
		float yaw_ {0.f};
		float pitch_ {0.f};

		quat rot_;
	};
}