#include "vec4.hh"

#include "mat4.hh"

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

	vec4 vec4::operator-() const
	{
		return {-x, -y, -z, -w};
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

	vec4 vec4::operator*(mat4 const& rhs) const
	{
		return {
			x * (rhs[0][0]) + y * (rhs[1][0]) + z * (rhs[2][0]) + w * (rhs[3][0]),
			x * (rhs[0][1]) + y * (rhs[1][1]) + z * (rhs[2][1]) + w * (rhs[3][1]),
			x * (rhs[0][2]) + y * (rhs[1][2]) + z * (rhs[2][2]) + w * (rhs[3][2]),
			x * (rhs[0][3]) + y * (rhs[1][3]) + z * (rhs[2][3]) + w * (rhs[3][3]),
		};
	}

	vec4& vec4::operator*=(mat4 const& rhs)
	{
		x = x * (rhs[0][0]) + y * (rhs[1][0]) + z * (rhs[2][0]) + w * (rhs[3][0]);
		y = x * (rhs[0][1]) + y * (rhs[1][1]) + z * (rhs[2][1]) + w * (rhs[3][1]);
		z = x * (rhs[0][2]) + y * (rhs[1][2]) + z * (rhs[2][2]) + w * (rhs[3][2]);
		w = x * (rhs[0][3]) + y * (rhs[1][3]) + z * (rhs[2][3]) + w * (rhs[3][3]);

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
		float len = sqrtf(x * x + y * y + z * z + w * w);

		return {x / len, y / len, z / len, w / len};
	}

	vec4 vec4::norm3() const
	{
		float len = sqrtf(x * x + y * y + z * z);

		return {x / len, y / len, z / len, 1.f};
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
		return x * x + y * y + z * z + w * w;
	}

	float vec4::len() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}
}