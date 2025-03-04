#pragma once

namespace vkb
{
	class window;

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
		context(window& win, vk::context& vk);
		~context();

		void update(double dt);
		void draw();

	private:
		window&      win_;
		vk::context& vk_;

		bool demo_ {false};

		double refresh {0.0};
		float  fps {0.f};
	};
}