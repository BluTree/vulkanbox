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

	namespace cam
	{
		class free;
	}
}

namespace vkb::ui
{
	class context
	{
	public:
		context(window& win, input_system& is, vk::context& vk, cam::free& cam);
		~context();

		void update(double dt);
		void draw();

	private:
		[[maybe_unused]] window&       win_;
		[[maybe_unused]] input_system& is_;
		[[maybe_unused]] vk::context&  vk_;
		[[maybe_unused]] cam::free&    cam_;

		bool demo_ {false};

		double   refresh {0.0};
		uint32_t fps {0};
		uint32_t disp_fps {0};
	};
}