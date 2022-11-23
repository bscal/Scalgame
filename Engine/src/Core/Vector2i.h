#pragma once

#include <stdint.h>
#include <raymath.h>

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
	bool Equals(Vector2i other) const;
};

int64_t  Vec2iPackInt64(Vector2i v);
Vector2i Vec2iUnpackInt64(int64_t packedVector);
Vector2i Vec2iOne();
Vector2i Vec2iZero();
Vector2i Vec2fToVec2i(Vector2 v);
Vector2  Vec2iToVec2f(Vector2i v);
