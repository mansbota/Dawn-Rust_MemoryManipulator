#pragma once

#include <math.h>

class Vec2
{
public:
	float x, y;

	Vec2(float x = 0.f, float y = 0.f) : x{ x }, y{ y } {}

	Vec2 operator+(const Vec2& other) { return { x + other.x, y + other.y }; }
	Vec2 operator-(const Vec2& other) { return { x - other.x, y - other.y }; }
	Vec2 operator*(const Vec2& other) { return { x * other.x, y * other.y }; }
	Vec2 operator/(const Vec2& other) { return { x / other.x, y / other.y }; }
	Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; }
	Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; }
	Vec2& operator*=(const Vec2& other) { x *= other.x; y *= other.y; }
	Vec2& operator/=(const Vec2& other) { x /= other.x; y /= other.y; }

	Vec2 operator+(float value) { return { x + value, y + value }; }
	Vec2 operator-(float value) { return { x - value, y - value }; }
	Vec2 operator*(float value) { return { x * value, y * value }; }
	Vec2 operator/(float value) { return { x / value, y / value }; }
	Vec2& operator+=(float value) { x += value, y += value; }
	Vec2& operator-=(float value) { x -= value, y -= value; }
	Vec2& operator*=(float value) { x *= value, y *= value; }
	Vec2& operator/=(float value) { x /= value, y /= value; }

	float distanceFrom(const Vec2& other) {
		return magnitude(*this - other);
	}

	static float magnitude(const Vec2& vector) {
		return sqrtf(vector.x * vector.x + vector.y * vector.y);
	}
};
