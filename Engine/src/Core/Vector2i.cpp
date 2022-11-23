#include "Vector2i.h"

int64_t Vec2iPackInt64(Vector2i v)
{
    return (int64_t)v.x << 32 | v.y;
}

Vector2i Vec2iUnpackInt64(int64_t packedVector)
{
    int y = (int)packedVector;
    int x = (int)(packedVector >> 32);
    return { x, y };
}

Vector2i Vec2iOne()
{
    return { 1, 1 };
}

Vector2i Vec2iZero()
{
    return { 0, 0 };
}

Vector2i Vec2fToVec2i(Vector2 v)
{
    return { (int)v.x, (int)v.y };
}

Vector2 Vec2iToVec2f(Vector2i v)
{
    return { (float)v.x, (float)v.y };
}

Vector2i Vector2i::Add(Vector2i o) const
{
    return { x + o.x, y + o.y };
}

Vector2i Vector2i::AddValue(int add) const
{
    return { x + add, y + add };
}

Vector2i Vector2i::Subtract(Vector2i o) const
{
    return { x - o.x, y - o.y };
}

Vector2i Vector2i::SubtractValue(int sub) const
{
    return { x - sub, y - sub };
}

// Calculate vector length
float Vector2i::Magnitude() const
{
    float result = sqrtf((float)((x * x) + (y * y)));
    return result;
}

// Calculate vector square length
int Vector2i::SqrMagnitude() const
{
    return (x * x) + (y * y);
}

// Calculate two vectors dot product
float Vector2i::Dot(Vector2i o) const
{
    float result = (float)(x * o.x + y * o.y);
    return result;
}

// Calculate distance between two vectors
float Vector2i::Distance(Vector2i o) const
{
    float result = sqrtf((float)((x - o.x) * (x - o.x) + (y - o.y) * (y - o.y)));
    return result;
}

// Calculate square distance between two vectors
int Vector2i::SqrDistance(Vector2i o) const
{
    int result = ((x - o.x) * (x - o.x) + (y - o.y) * (y - o.y));
    return result;
}

// Calculate angle from two vectors
float Vector2i::Angle(Vector2 o) const
{
    float result = atan2f((float)o.y, (float)o.x) - atan2f((float)y, (float)x);
    return result;
}

// Scale vector (multiply by value)
Vector2i Vector2i::Scale(int scale) const
{
    return { x * scale, y * scale };
}

// Multiply vector by vector
Vector2i Vector2i::Multiply(Vector2i o) const
{
    return { x * o.x, y * o.y };
}

// Negate vector
Vector2i Vector2i::Negate() const
{
    return { -x, -y };
}

Vector2i Vector2i::Min(Vector2i min) const
{
    Vector2i result;
    result.x = (x < min.x) ? min.x : x;
    result.y = (y < min.y) ? min.y : y;
    return result;
}

Vector2i Vector2i::Max(Vector2i max) const
{
    Vector2i result;
    result.x = (x > max.x) ? max.x : x;
    result.y = (y > max.y) ? max.y : y;
    return result;
}

// Divide vector by vector
Vector2i Vector2i::Divide(Vector2i o) const
{
    return { x / o.x, y / o.y };
}

bool Vector2i::Equals(Vector2i other) const
{
    return (x == other.x) && (y == other.y);
}
