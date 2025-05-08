#include "quat.hh"

#include "mat4.hh"

#include <math.h>

namespace vkb
{
	quat quat::angle_axis(vec4 axis, float angle)
	{
		float a_sin {sinf(angle / 2.f)};
		return {
			cosf(angle / 2.f),
			a_sin * axis.x,
			a_sin * axis.y,
			a_sin * axis.z,
		};
	}

	quat quat::euler(vec4 euler)
	{
		float x_cos {cosf(euler.x / 2.f)};
		float y_cos {cosf(euler.y / 2.f)};
		float z_cos {cosf(euler.z / 2.f)};
		float x_sin {sinf(euler.x / 2.f)};
		float y_sin {sinf(euler.y / 2.f)};
		float z_sin {sinf(euler.z / 2.f)};

		return {
			(x_cos * y_cos * z_cos) - (x_sin * y_sin * z_sin),
			(x_sin * y_cos * z_cos) + (x_cos * y_sin * z_sin),
			(x_cos * y_sin * z_cos) + (x_sin * y_cos * z_sin),
			(x_cos * y_cos * z_sin) - (x_sin * y_sin * z_cos),
		};
	}

	quat quat::operator*(quat quat) const
	{
		return {
			w * quat.w - x * quat.x - y * quat.y - z * quat.z,
			w * quat.x + x * quat.w - y * quat.z + z * quat.y,
			w * quat.y + x * quat.z + y * quat.w - z * quat.x,
			w * quat.z - x * quat.y + y * quat.x + z * quat.w,
		};
	}

	quat::operator mat4() const
	{
		float res[4][4] {
			{1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z,     2 * x * z + 2 * w * y,     0},
			{2 * x * y + 2 * w * z,     1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x,     0},
			{2 * x * z - 2 * w * y,     2 * y * z + 2 * w * x,     1 - 2 * x * x - 2 * y * y, 0},
			{0,						 0,						 0,						 1}
        };
		return {res};
	}

	vec4 quat::rotate(vec4 vec) const
	{
		vec4 qvec {x, y, z, 0};

		return qvec * (2 * qvec.dot(vec)) + vec * (w * w - qvec.dot(qvec)) +
		       (qvec.cross3(vec)) * 2 * w;
	}

	quat quat::inverse() const
	{
		float sq_len = w * w + x * x + y * y + z * z;
		return {w * sq_len, -x * sq_len, -y * sq_len, -z * sq_len};
	}

}