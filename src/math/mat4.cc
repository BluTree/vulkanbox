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
			1.f, 0.f, 0.f, trans.x,
			0.f, 1.f, 0.f, trans.y,
			0.f, 0.f, 1.f, trans.z,
			0.f, 0.f, 0.f, 1.f,
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
			0.f,      0.f,       nf,   nf2,
			0.f,      0.f,       -1.f, 0.f
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
			2 / (r - l), 0.f,               0.f, rl,
			0.f,              -2 / (t - b), 0.f, tb,
			0.f,              0.f,          nf,  nf2,
			0.f,              0.f,          0.f, 1.f
		};
		// clang-format on
	}

	mat4 mat4::look_at(vec4 eye, vec4 at, vec4 up)
	{
		vec4 z = (at - eye).norm3();
		vec4 x = z.cross3(up).norm3();
		vec4 y = x.cross3(z);

		// clang-format off
		return {
			x.x,  x.y,  x.z,  -x.dot3(eye),
			y.x,  y.y,  y.z,  -y.dot3(eye),
			-z.x, -z.y, -z.z, z.dot3(eye),
			0.f, 0.f, 0.f, 1.f
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
		for (uint32_t i {0}; i < 16; ++i)
			arr_[i % 4][i / 4] = arr[i];
	}

	mat4::mat4(std::initializer_list<float> arr)
	{
		for (uint32_t i {0}; i < 16; ++i)
			arr_[i % 4][i / 4] = arr.begin()[i];
	}

	float* mat4::operator[](uint8_t i) &
	{
		return arr_[i];
	}

	float const* mat4::operator[](uint8_t i) const&
	{
		return arr_[i];
	}

	mat4 mat4::operator*(mat4 const& other)
	{
		mat4 res;

		for (uint32_t i {0}; i < 4; ++i)
		{
			for (uint32_t j {0}; j < 4; ++j)
			{
				float val {0};
				for (uint32_t k {0}; k < 4; ++k)
					val += (*this)[k][j] * other[i][k];
				res[i][j] = val;
			}
		}
		return res;
	}
}