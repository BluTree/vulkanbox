#pragma once

#include <stdint.h>

namespace vkb
{
	class window;
	class input_system;

	namespace vk
	{
		class context;
	}
}

namespace vkb::ui
{
	class context
	{
	public:
		context(window& win, input_system& is, vk::context& vk);
		~context();

		void update(double dt);
		void draw();

	private:
		window&       win_;
		input_system& is_;
		vk::context&  vk_;

		bool demo_ {false};

		double   refresh {0.0};
		uint32_t fps {0};
		uint32_t disp_fps {0};
	};
}