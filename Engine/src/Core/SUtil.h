#pragma once

#include "Core.h"
#include "SString.h"
#include "SHash.hpp"
#include "Vector2i.h"

#include "Structures/SList.h"

Vector4 Vec4Add(const Vector4& v0, const Vector4& v1);

int IModNegative(int a, int b);

template<typename T>
struct DefaultHasher
{
	[[nodiscard]] constexpr uint64_t operator()(const T* key) const noexcept
	{
		const uint8_t* const data = (const uint8_t* const)(key);
		return FNVHash64(data, sizeof(T));
	}
};

template<typename T>
struct DefaultEquals
{
	[[nodiscard]] bool operator()(const T* k1, const T* k2) const noexcept
	{
		return *k1 == *k2;
	}
};

_FORCE_INLINE_ uint64_t
SStringHash(const SString* key)
{
	const uint8_t* const data = (const uint8_t* const)key->Data();
	return FNVHash64(data, key->Length);
}

_FORCE_INLINE_ uint64_t
SStringViewHash(const SStringView* key)
{
	const uint8_t* const data = (const uint8_t* const)key->Str;
	return FNVHash64(data, key->Length);
}

struct Vector2iHasher
{
	[[nodiscard]] _FORCE_INLINE_ constexpr size_t operator()(const Vector2i* v) const noexcept
	{
		return FNVHash64((const uint8_t*)v, sizeof(Vector2i));
	}
};

struct Vector2iEquals
{
	_FORCE_INLINE_ bool operator()(const Vector2i* lhs, const Vector2i* rhs) const noexcept
	{
		return lhs->Equals(*rhs);
	}
};

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
	size_t res = (size + static_cast<size_t>(alignment - 1)) & static_cast<size_t>(~(alignment - 1));
	return res;
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
