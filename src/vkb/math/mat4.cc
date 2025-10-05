#include "mat4.hh"

#include "vec4.hh"

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
		res[0][1] = axis.y * axis.x * inv_cos - axis.z * a_sin;
		res[0][2] = axis.z * axis.x * inv_cos + axis.y * a_sin;

		res[1][0] = axis.x * axis.y * inv_cos + axis.z * a_sin;
		res[1][1] = axis.y * axis.y * inv_cos + a_cos;
		res[1][2] = axis.z * axis.y * inv_cos - axis.x * a_sin;

		res[2][0] = axis.x * axis.z * inv_cos - axis.y * a_sin;
		res[2][1] = axis.y * axis.z * inv_cos + axis.x * a_sin;
		res[2][2] = axis.z * axis.z * inv_cos + a_cos;

		return res;
	}

	mat4 mat4::translate(vec4 trans)
	{
		// clang-format off
		return {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			trans.x, trans.y, trans.z, 1.f,
		};
		// clang-format on
	}

	mat4 mat4::persp_proj(float near, float far, float asp_ratio, float fov,
	                      fov_axis axis)
	{
		float fov_tan = tanf(fov / 2.f);
		float r;
		float t;
		if (axis == fov_axis::y)
		{
			t = near * fov_tan;
			r = t * asp_ratio;
		}
		else
		{
			r = near * fov_tan;
			t = r / asp_ratio;
		}

		float nf = -(far) / (far - near);
		float nf2 = -(far * near) / (far - near);

		// clang-format off
		return {
			near / r, 0.f,       0.f,  0.f,
			0.f,      -near / t, 0.f,  0.f,
			0.f,      0.f,       nf,   -1.f,
			0.f,      0.f,       nf2, 0.f
		};
		// clang-format on
	}

	mat4 mat4::ortho_proj(float near, float far, float l, float r, float t, float b)
	{
		float rl = -(r + l) / (r - l);
		float tb = -(t + b) / (t - b);
		float nf = -2 / (far - near);
		float nf2 = -(far + near) / (far - near);
		// clang-format off
		return {
			2 / (r - l), 0.f,               0.f, 0.f,
			0.f,              -2 / (t - b), 0.f, 0.f,
			0.f,              0.f,          nf,  0.f,
			rl,              tb,          nf2, 1.f
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

	mat4 mat4::operator*(mat4 const& other) const
	{
		mat4 res;

		for (uint32_t i {0}; i < 4; ++i)
		{
			for (uint32_t j {0}; j < 4; ++j)
			{
				res[i][j] += (*this)[i][0] * other[0][j];
				res[i][j] += (*this)[i][1] * other[1][j];
				res[i][j] += (*this)[i][2] * other[2][j];
				res[i][j] += (*this)[i][3] * other[3][j];
			}
		}
		return res;
	}

	mat4 mat4::transpose() const
	{
		mat4 res;

		return {
			(*this)[0][0], (*this)[1][0], (*this)[2][0], (*this)[3][0],
			(*this)[0][1], (*this)[1][1], (*this)[2][1], (*this)[3][1],
			(*this)[0][2], (*this)[1][2], (*this)[2][2], (*this)[3][2],
			(*this)[0][3], (*this)[1][3], (*this)[2][3], (*this)[3][3],
		};
	}
}