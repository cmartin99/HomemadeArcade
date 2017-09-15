#pragma once
#include "TdBase.h"

namespace eng {

struct TdPoint2
{
	int32 x, y;
	TdPoint2() {};
	TdPoint2(int32 i) : x(i), y(i) {};
	TdPoint2(int32 x, int32 y) : x(x), y(y) {};
	TdPoint2(const TdPoint2& p) : x(p.x), y(p.y) {};
	TdPoint2(const Vector2& p) : x((int32)p.x), y((int32)p.y) {};
	int GetSum() const { return x + y; }
	int GetProduct() const { return x * y; }
	void Negate() { x = -x; y = -y; }
	TdPoint2 Left(int32 i) const { return TdPoint2(x - i, y); }
	TdPoint2 Right(int32 i) const { return TdPoint2(x + i, y); }
	TdPoint2 Up(int32 i) const { return TdPoint2(x, y + i); }
	TdPoint2 Down(int32 i) const { return TdPoint2(x, y - i); }
	TdPoint2& operator= (const TdPoint2& p) { x = p.x; y = p.y; return *this; }
	int64 sum() const { return x + y; }
	int64 prod() const { return x * y; }
	int64 volume() const { return x * y; }
};

ALWAYS_INLINE TdPoint2 TdPoint2Min(TdPoint2 p1, TdPoint2 p2) { return TdPoint2(p1.x < p2.x ? p1.x : p2.x, p1.y < p2.y ? p1.y : p2.y); }
ALWAYS_INLINE TdPoint2 TdPoint2Max(TdPoint2 p1, TdPoint2 p2) { return TdPoint2(p1.x > p2.x ? p1.x : p2.x, p1.y > p2.y ? p1.y : p2.y); }

ALWAYS_INLINE bool operator==(const TdPoint2& l, const TdPoint2& r) { return l.x == r.x && l.y == r.y; }
ALWAYS_INLINE bool operator!=(const TdPoint2& l, const TdPoint2& r) { return l.x != r.x || l.y != r.y; }
ALWAYS_INLINE TdPoint2 operator+(const TdPoint2& l, const TdPoint2& r) { return TdPoint2(l.x + r.x, l.y + r.y); }
ALWAYS_INLINE TdPoint2 operator-(const TdPoint2& l, const TdPoint2& r) { return TdPoint2(l.x - r.x, l.y - r.y); }
ALWAYS_INLINE TdPoint2 operator*(const TdPoint2& l, const TdPoint2& r) { return TdPoint2(l.x * r.x, l.y * r.y); }
ALWAYS_INLINE TdPoint2 operator/(const TdPoint2& l, const TdPoint2& r) { return TdPoint2(l.x / r.x, l.y / r.y); }
ALWAYS_INLINE TdPoint2 operator+(const TdPoint2& l, int r) { return TdPoint2(l.x + r, l.y + r); }
ALWAYS_INLINE TdPoint2 operator-(const TdPoint2& l, int r) { return TdPoint2(l.x - r, l.y - r); }
ALWAYS_INLINE TdPoint2 operator*(const TdPoint2& l, int r) { return TdPoint2(l.x * r, l.y * r); }
ALWAYS_INLINE TdPoint2 operator/(const TdPoint2& l, int r) { return TdPoint2(l.x / r, l.y / r); }
ALWAYS_INLINE void operator-=(TdPoint2& l, const TdPoint2& r) { l.x -= r.x; l.y -= r.y; }
ALWAYS_INLINE void operator+=(TdPoint2& l, const TdPoint2& r) { l.x += r.x; l.y += r.y; }
ALWAYS_INLINE void operator*=(TdPoint2& l, const TdPoint2& r) { l.x *= r.x; l.y *= r.y; }
ALWAYS_INLINE void operator/=(TdPoint2& l, const TdPoint2& r) { l.x /= r.x; l.y /= r.y; }

struct TdPoint3
{
	int32 x, y, z;
	TdPoint3() {};
	TdPoint3(int32 x) : x(x), y(x), z(x) {};
	TdPoint3(int32 x, int32 y, int32 z) : x(x), y(y), z(z) {};
	TdPoint3(const TdPoint3& p) : x(p.x), y(p.y), z(p.z) {};
	TdPoint3(const Vector3& p) : x((int32)p.x), y((int32)p.y), z((int32)p.z) {};
	uint64 GetSum() const { return x + y + z; }
	uint64 GetProduct() const { return x * y * z; }
	void Negate() { x = -x; y = -y; z = -z; }
	bool IsMax() const { return x == INT_MAX && y == INT_MAX && z == INT_MAX; }
	TdPoint3 Left(int32 i) const { return TdPoint3(x - i, y, z); }
	TdPoint3 Forward(int32 i) const { return TdPoint3(x, y, z - i); }
	TdPoint3 Right(int32 i) const { return TdPoint3(x + i, y, z); }
	TdPoint3 Backward(int32 i) const { return TdPoint3(x, y, z + i); }
	TdPoint3 Up(int32 i) const { return TdPoint3(x, y + i, z); }
	TdPoint3 Down(int32 i) const { return TdPoint3(x, y - i, z); }
	TdPoint3& operator= (const TdPoint3& p) { x = p.x; y = p.y; z = p.z; return *this; }
	int64 sum() const { return x + y + z; }
	int64 prod() const { return x * y * z; }
	int64 volume() const { return x * y * z; }
};

ALWAYS_INLINE bool operator==(const TdPoint3& l, const TdPoint3& r) { return l.x == r.x && l.y == r.y && l.z == r.z; }
ALWAYS_INLINE bool operator!=(const TdPoint3& l, const TdPoint3& r) { return l.x != r.x || l.y != r.y || l.z != r.z; }
ALWAYS_INLINE TdPoint3 operator+(const TdPoint3& l, const TdPoint3& r) { return TdPoint3(l.x + r.x, l.y + r.y, l.z + r.z); }
ALWAYS_INLINE TdPoint3 operator-(const TdPoint3& l, const TdPoint3& r) { return TdPoint3(l.x - r.x, l.y - r.y, l.z - r.z); }
ALWAYS_INLINE TdPoint3 operator*(const TdPoint3& l, const TdPoint3& r) { return TdPoint3(l.x * r.x, l.y * r.y, l.z * r.z); }
ALWAYS_INLINE TdPoint3 operator/(const TdPoint3& l, const TdPoint3& r) { return TdPoint3(l.x / r.x, l.y / r.y, l.z / r.z); }
ALWAYS_INLINE TdPoint3 operator+(const TdPoint3& l, int r) { return TdPoint3(l.x + r, l.y + r, l.z + r); }
ALWAYS_INLINE TdPoint3 operator-(const TdPoint3& l, int r) { return TdPoint3(l.x - r, l.y - r, l.z - r); }
ALWAYS_INLINE TdPoint3 operator*(const TdPoint3& l, int r) { return TdPoint3(l.x * r, l.y * r, l.z * r); }
ALWAYS_INLINE TdPoint3 operator/(const TdPoint3& l, int r) { return TdPoint3(l.x / r, l.y / r, l.z / r); }
ALWAYS_INLINE void operator-=(TdPoint3& l, const TdPoint3& r) { l.x -= r.x; l.y -= r.y; l.z -= r.z; }
ALWAYS_INLINE void operator+=(TdPoint3& l, const TdPoint3& r) { l.x += r.x; l.y += r.y; l.z += r.z; }
ALWAYS_INLINE void operator*=(TdPoint3& l, const TdPoint3& r) { l.x *= r.x; l.y *= r.y; l.z *= r.z; }
ALWAYS_INLINE void operator/=(TdPoint3& l, const TdPoint3& r) { l.x /= r.x; l.y /= r.y; l.z /= r.z; }

}