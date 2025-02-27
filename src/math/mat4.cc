#include "mat4.hh"

#include <math.h>
#include <string.h>

namespace vkb
{
	// clang-format off

	mat4 mat4::identity {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	// clang-format on

	mat4 mat4::scale(vec4 scale)
	{
		// clang-format off
		return {
			scale.x, 0.f,     0.f,     0.f,
			0.f,     scale.y, 0.f,     0.f,
			0.f,     0.f,     scale.z, 0.f,
			0.f,     0.f,     0.f,     1.f,
		};
		// clang-format on
	}

	mat4 mat4::rotate(vec4 axis, float angle)
	{
		float a_cos {cosf(angle)};
		float a_sin {sinf(angle)};
		float inv_cos {1 - a_cos};

		mat4 res {mat4::identity};

		res[0][0] = axis.x * axis.x * inv_cos + a_cos;
		res[1][0] = axis.x * axis.y * inv_cos + axis.z * a_sin;
		res[2][0] = axis.x * axis.z * inv_cos - axis.y * a_sin;

		res[0][1] = axis.y * axis.x * inv_cos - axis.z * a_sin;
		res[1][1] = axis.y * axis.y * inv_cos + a_cos;
		res[2][1] = axis.y * axis.z * inv_cos + axis.x * a_sin;

		res[0][2] = axis.z * axis.x * inv_cos + axis.y * a_sin;
		res[1][2] = axis.z * axis.y * inv_cos - axis.x * a_sin;
		res[2][2] = axis.z * axis.z * inv_cos + a_cos;

		return res;
	}

	mat4 mat4::translate(vec4 trans)
	{
		// clang-format off
		return {
			0.f, 0.f, 0.f, trans.x,
			0.f, 0.f, 0.f, trans.y,
			0.f, 0.f, 0.f, trans.z,
			0.f, 0.f, 0.f, 1.f,
		};
		// clang-format on
	}

	mat4::mat4()
	{
		memset(arr_, 0, 16 * sizeof(float));
	}

	mat4::mat4(float arr[4][4])
	{
		memcpy(arr_, arr, 16 * sizeof(float));
	}

	mat4::mat4(float arr[16])
	{
		memcpy(arr_, arr, 16 * sizeof(float));
	}

	mat4::mat4(std::initializer_list<float> arr)
	{
		memcpy(arr_, arr.begin(), 16 * sizeof(float));
	}

	float* mat4::operator[](uint8_t i) &
	{
		return arr_[i];
	}

	float const* mat4::operator[](uint8_t i) const&
	{
		return arr_[i];
	}
}