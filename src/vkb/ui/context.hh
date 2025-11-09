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
		[[maybe_unused]] window&       win_;
		[[maybe_unused]] input_system& is_;
		[[maybe_unused]] vk::context&  vk_;

		[[maybe_unused]] bool demo_ {false};

		[[maybe_unused]] double   refresh {0.0};
		[[maybe_unused]] uint32_t fps {0};
		[[maybe_unused]] uint32_t disp_fps {0};
	};
}