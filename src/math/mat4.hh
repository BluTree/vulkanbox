#pragma once

#include <stdint.h>

#include <initializer_list.hh>

#include "vec4.hh"

namespace vkb
{
	enum class fov_axis
	{
		x,
		y
	};

	class mat4
	{
	public:
		static mat4 identity;
		static mat4 scale(vec4 scale);
		static mat4 rotate(vec4 axis, float angle);
		static mat4 translate(vec4 trans);

		static mat4 persp_proj(float near, float far, float asp_ratio, float fov,
		                       fov_axis axis = fov_axis::y);
		static mat4 ortho_proj(float near, float far, float l, float r, float t, float b);
		static mat4 look_at(vec4 eye, vec4 at, vec4 up);

		mat4();
		mat4(float arr[4][4]);
		mat4(float arr[16]);
		mat4(std::initializer_list<float> arr);

		float*       operator[](uint8_t i) &;
		float const* operator[](uint8_t i) const&;

		mat4 operator*(mat4 const& other);

		mat4 transpose() const;

		float const* operator[](uint8_t i) const&& = delete;

	private:
		float arr_[4][4];
	};

}