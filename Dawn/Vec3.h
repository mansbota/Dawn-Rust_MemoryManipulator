#pragma once

#include "Vec2.h"

class Vec3 : public Vec2
{
public:
	float z;

	Vec3(float x = 0.f, float y = 0.f, float z = 0.f) : Vec2{ x, y }, z{ z } {}

	Vec3 operator+(const Vec3& other) { return { x + other.x, y + other.y, z + other.z}; }
	Vec3 operator-(const Vec3& other) { return { x - other.x, y - other.y, z - other.z }; }
	Vec3 operator*(const Vec3& other) { return { x * other.x, y * other.y, z * other.z }; }
	Vec3 operator/(const Vec3& other) { return { x / other.x, y / other.y, z / other.z }; }
	Vec3& operator+=(const Vec3& other) { x += other.x; y += other.y, z += other.z; }
	Vec3& operator-=(const Vec3& other) { x -= other.x; y -= other.y, z -= other.z; }
	Vec3& operator*=(const Vec3& other) { x *= other.x; y *= other.y, z *= other.z; }
	Vec3& operator/=(const Vec3& other) { x /= other.x; y /= other.y, z /= other.z; }

	Vec3 operator+(float value) { return { x + value, y + value, z + value }; }
	Vec3 operator-(float value) { return { x - value, y - value, z - value }; }
	Vec3 operator*(float value) { return { x * value, y * value, z * value }; }
	Vec3 operator/(float value) { return { x / value, y / value, z / value }; }
	Vec3& operator+=(float value) { x += value; y += value, z += value; }
	Vec3& operator-=(float value) { x -= value; y -= value, z -= value; }
	Vec3& operator*=(float value) { x *= value; y *= value, z *= value; }
	Vec3& operator/=(float value) { x /= value; y /= value, z /= value; }

	float distanceFrom(const Vec3& other) {
		return magnitude(*this - other);
	}

	static float magnitude(const Vec3& vector) {
		return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	}
};

