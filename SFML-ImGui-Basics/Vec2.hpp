#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>

template <typename T>
class Vec2
{
public:
	T x = 0;
	T y = 0;

	Vec2() = default;

	Vec2(T xin, T yin) : x(xin), y(yin) {}

	Vec2(const sf::Vector2<T>& vec) : x(vec.x), y(vec.y) {}

	operator sf::Vector2<T>()
	{
		return sf::Vector2<T>(x, y);
	}

	Vec2 operator + (const Vec2& rhs) const
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}

	Vec2 operator - (const Vec2& rhs) const
	{
		return Vec2(x - rhs.x, y - rhs.y);
	}

	Vec2 operator * (const T val) const
	{
		return Vec2(x * val, y * val);
	}

	Vec2 operator / (const T val) const
	{
		return Vec2(x / val, y / val);
	}

	bool operator == (const Vec2& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}

	bool operator != (const Vec2& rhs) const
	{
		return !(*this == rhs);
	}

	void operator += (const Vec2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
	}

	void operator -= (const Vec2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
	}

	void operator *= (const T val)
	{
		x *= val;
		y *= val;
	}

	void operator /= (const T val)
	{
		x /= val;
		y /= val;
	}

	float dist(const Vec2& rhs) const
	{
		T dx = x - rhs.x;
		T dy = y - rhs.y;
		return std::sqrt(static_cast<float>(dx * dx + dy * dy));
	}

	float angle(const Vec2& rhs) const
	{
		float diffY = y - rhs.y;
		float diffX = x - rhs.x;
		return atan2f(diffY, diffX);
	}

	Vec2 normalize(const Vec2& rhs) const
	{
		float diffY = y - rhs.y;
		float diffX = x - rhs.x;
		float length = sqrt((diffY * diffY) + (diffX * diffX));

		return Vec2(diffX / length, diffY / length);
	}
};

using Vec2f = Vec2<float>;
