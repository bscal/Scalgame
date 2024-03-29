#pragma once

#include "Core.h"
#include "SString.h"
#include "SHash.hpp"
#include "Vector2i.h"

#include "Structures/SList.h"

Vector4 Vec4Add(const Vector4& v0, const Vector4& v1);

int IModNegative(int a, int b);

struct DefaultHasher
{
	[[nodiscard]] _FORCE_INLINE_ constexpr uint32_t operator()(const void* key, size_t size) const noexcept
	{
		return FNVHash32((const uint8_t*)key, size);
	}
};

_FORCE_INLINE_ uint64_t
SStringHash(const SString* key)
{
	const uint8_t* const data = (const uint8_t* const)key->Data();
	return FNVHash64(data, key->Length);
}

_FORCE_INLINE_ constexpr size_t
AlignPowTwo64(size_t num)
{
	if (num == 0) return 0;

	size_t power = 1;
	while (num >>= 1) power <<= 1;
	return power;
}

_FORCE_INLINE_ constexpr uint32_t
AlignPowTwo32(uint32_t num)
{
	if (num == 0) return 0;

	uint32_t power = 1;
	while (num >>= 1) power <<= 1;
	return power;
}

_FORCE_INLINE_ constexpr size_t
AlignPowTwo64Ceil(size_t x)
{
	if (x <= 1) return 1;
	size_t power = 2;
	--x;
	while (x >>= 1) power <<= 1;
	return power;
}

_FORCE_INLINE_ constexpr uint32_t
AlignPowTwo32Ceil(uint32_t x)
{
	if (x <= 1) return 1;
	uint32_t power = 2;
	--x;
	while (x >>= 1) power <<= 1;
	return power;
}

_FORCE_INLINE_ constexpr bool
IsPowerOf2_32(uint32_t num)
{
	return (num > 0 && ((num & (num - 1)) == 0));
}

_FORCE_INLINE_ constexpr size_t
AlignSize(size_t size, size_t alignment)
{
	return (size + (alignment - 1)) & ~(alignment - 1);
}

_FORCE_INLINE_ constexpr bool
IsPowerOf2(size_t num)
{
	return (num > 0ULL && ((num & (num - 1ULL)) == 0ULL));
}

struct World;


// bresenhams line algorithm int
template<typename OnVisit>
void 
Raytrace2DInt(World* world, int x0, int y0, int x1, int y1)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int x = x0;
	int y = y0;
	int n = 1 + dx + dy;
	int xInc = (x1 > x0) ? 1 : -1;
	int yInc = (y1 > y0) ? 1 : -1;
	int error = dx - dy;

	dx *= 2;
	dy *= 2;
	for (; n > 0; --n)
	{
		if (OnVisit{}(world, x, y))
			break;

		if (error > 0)
		{
			x += xInc;
			error -= dy;
		}
		else
		{
			y += yInc;
			error += dx;
		}
	}
}

// Temporary list of tile coordinates
SList<Vector2i>
QueryTilesRect(World* world, Vector2i start, Vector2i end);

// Returns a temporary array of tile positions
SList<Vector2i>
QueryTilesRadius(World* world, Vector2i center, float radius);

Color IntToColor(int colorInt);

/*
*	Split string into multiple strings
*
*	buffer - buffer to put the split string text
*	splitBuffer - buffer to store split char*'s
*/
void TextSplitBuffered(const char* text, char delimiter, int* count, char* buffer, int bufferLength, char** splitBuffer, int splitBufferLength);

typedef enum
{
	STR2INT_SUCCESS,
	STR2INT_OVERFLOW,
	STR2INT_UNDERFLOW,
	STR2INT_INCONVERTIBLE
} STR2INT;

/* Convert string s to int out
 * 
 * out cannot be null
 * cannot be empty
 * no leading whitespaces
 * no trailing chars
 */
STR2INT Str2Int(int* out, const char* s, int base);

STR2INT Str2UInt(uint32_t* out, const char* s, int base);

int FastAtoi(const char* str);
