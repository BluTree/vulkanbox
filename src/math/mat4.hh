#pragma once

#include <stdint.h>

#include <initializer_list.hh>

#include "vec4.hh"

namespace vkb
{
	class mat4
	{
	public:
		static mat4 identity;
		static mat4 scale(vec4 scale);
		static mat4 rotate(vec4 axis, float angle);
		static mat4 translate(vec4 trans);

		mat4();
		mat4(float arr[4][4]);
		mat4(float arr[16]);
		mat4(std::initializer_list<float> arr);

		float*       operator[](uint8_t i) &;
		float const* operator[](uint8_t i) const&;

		float const* operator[](uint8_t i) const&& = delete;

	private:
		float arr_[4][4];
	};

}