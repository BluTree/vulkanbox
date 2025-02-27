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

	vec4 quat::rotate(vec4 vec)
	{
		vec4 qvec {x, y, z, 0};

		return qvec * (2 * qvec.dot(vec)) + vec * (w * w - qvec.dot(qvec)) +
		       (qvec.cross3(vec)) * 2 * w;
	}

	quat::operator mat4() const
	{
		mat4 res;

		return res;
	}
}