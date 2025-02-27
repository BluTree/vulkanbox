#pragma once

namespace vkb
{
	struct vec4
	{
		vec4  operator+(vec4 rhs) const;
		vec4& operator+=(vec4 rhs);
		vec4  operator+(float rhs) const;
		vec4& operator+=(float rhs);

		vec4  operator-(vec4 rhs) const;
		vec4& operator-=(vec4 rhs);
		vec4  operator-(float rhs) const;
		vec4& operator-=(float rhs);

		vec4  operator*(vec4 rhs) const;
		vec4& operator*=(vec4 rhs);
		vec4  operator*(float rhs) const;
		vec4& operator*=(float rhs);

		vec4  operator/(vec4 rhs) const;
		vec4& operator/=(vec4 rhs);
		vec4  operator/(float rhs) const;
		vec4& operator/=(float rhs);

		vec4 cross3(vec4 vec) const;

		vec4  norm() const;
		float dot(vec4 vec) const;
		float dot3(vec4 vec) const;

		float sq_len() const;
		float len() const;

		float x {0.f};
		float y {0.f};
		float z {0.f};
		float w {0.f};
	};

}