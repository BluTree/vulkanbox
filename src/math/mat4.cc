#include "mat4.hh"

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