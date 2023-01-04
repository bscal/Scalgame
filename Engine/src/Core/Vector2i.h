#pragma once

#include "raylib/src/raylib.h"

#include <stdint.h>
#include <functional>

struct Vector2i
{
	int x;
	int y;

	Vector2i Add(Vector2i o) const;
    Vector2i AddValue(int value) const;
    Vector2i Subtract(Vector2i o) const;
    Vector2i SubtractValue(int value) const;
    Vector2i Scale(int value) const;
    Vector2i Multiply(Vector2i o) const;
    Vector2i Divide(Vector2i o) const;
    float Magnitude() const;
    int SqrMagnitude() const;
    float Dot(Vector2i o) const;
    float Distance(Vector2i o) const;
    int SqrDistance(Vector2i o) const;
    float Angle(Vector2 o) const;
    Vector2i Negate() const;
    Vector2i Min(Vector2i min) const;
    Vector2i Max(Vector2i max) const;

    inline Vector2 AsVec2() const { return { (float)x, (float)y }; }
    inline bool Equals(Vector2i other) const { return (x == other.x && y == other.y); }
};

typedef Vector2i ChunkCoord;
typedef Vector2i TileCoord;

int64_t  Vec2iPackInt64(Vector2i v);
Vector2i Vec2iUnpackInt64(int64_t packedVector);
Vector2i Vec2iOne();
Vector2i Vec2iZero();
Vector2i Vec2iUp();
Vector2i Vec2iDown();
Vector2i Vec2iLeft();
Vector2i Vec2iRight();
Vector2i Vec2fToVec2i(Vector2 v);

template<>
struct std::hash<Vector2i>
{
    std::size_t operator()(const Vector2i& v) const noexcept
    {
        std::size_t i0 = std::hash<int>{}(v.x);
        std::size_t i1 = std::hash<int>{}(v.y);
        return i0 ^ (i1 << 1);
    }
};

struct Vector2iHasher
{
    std::size_t operator()(const Vector2i& v) const noexcept
    {
        std::size_t i0 = std::hash<int>{}(v.x);
        std::size_t i1 = std::hash<int>{}(v.y);
        return i0 ^ (i1 << 1);
    }
};

struct Vector2iEquals
{
    bool operator()(const Vector2i& lhs, const Vector2i& rhs) const noexcept
    {
        return lhs.Equals(rhs);
    }
};
