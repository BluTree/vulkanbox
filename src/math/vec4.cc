#include "vec4.hh"

#include <math.h>

namespace vkb
{
	vec4 vec4::operator+(vec4 rhs) const
	{
		return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
	}

	vec4& vec4::operator+=(vec4 rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;

		return *this;
	}

	vec4 vec4::operator+(float rhs) const
	{
		return {x + rhs, y + rhs, z + rhs, w + rhs};
	}

	vec4& vec4::operator+=(float rhs)
	{
		x += rhs;
		y += rhs;
		z += rhs;
		w += rhs;

		return *this;
	}

	vec4 vec4::operator-(vec4 rhs) const
	{
		return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
	}

	vec4& vec4::operator-=(vec4 rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;

		return *this;
	}

	vec4 vec4::operator-(float rhs) const
	{
		return {x - rhs, y - rhs, z - rhs, w - rhs};
	}

	vec4& vec4::operator-=(float rhs)
	{
		x -= rhs;
		y -= rhs;
		z -= rhs;
		w -= rhs;

		return *this;
	}

	vec4 vec4::operator*(vec4 rhs) const
	{
		return {x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
	}

	vec4& vec4::operator*=(vec4 rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		w *= rhs.w;

		return *this;
	}

	vec4 vec4::operator*(float rhs) const
	{
		return {x * rhs, y * rhs, z * rhs, w * rhs};
	}

	vec4& vec4::operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;

		return *this;
	}

	vec4 vec4::operator/(vec4 rhs) const
	{
		return {x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
	}

	vec4& vec4::operator/=(vec4 rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		w /= rhs.w;

		return *this;
	}

	vec4 vec4::operator/(float rhs) const
	{
		return {x / rhs, y / rhs, z / rhs, w / rhs};
	}

	vec4& vec4::operator/=(float rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;

		return *this;
	}

	vec4 vec4::cross3(vec4 vec) const
	{
		return {
			y * vec.z - z * vec.y,
			z * vec.x - x * vec.z,
			x * vec.y - y * vec.x,
			w,
		};
	}

	vec4 vec4::norm() const
	{
		float len = sqrtf(x * x + y * y + z + z + w * w);

		return {x / len, y / len, z / len, w / len};
	}

	float vec4::dot(vec4 vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
	}

	float vec4::dot3(vec4 vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	float vec4::sq_len() const
	{
		return x * x + y * y + z + z + w * w;
	}

	float vec4::len() const
	{
		return sqrtf(x * x + y * y + z + z + w * w);
	}
}