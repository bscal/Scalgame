#include "Core.h"

#include <assert.h>

internal void SCrash()
{
	assert(false);
}

MemorySizeData FindMemSize(uint64_t size)
{
	const uint64_t gb = 1024 * 1024 * 1024;
	const uint64_t mb = 1024 * 1024;
	const uint64_t kb = 1024;

	if (size > gb)
		return { (float)size / (float)gb, 'G' };
	else if (size > mb)
		return { (float)size / (float)mb, 'M' };
	else if (size > kb)
		return { (float)size / (float)kb, 'K' };
	else
		return { (float)size, ' ' };
}

bool Vector2i::operator == (const Vector2i& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool Vector2i::operator != (const Vector2i& rhs) const
{
	return !(*this == rhs);
}

int64_t Vector2i::ToInt64() const
{
	int64_t packedVec = ((size_t(x) & 0xffL) << 32);
	packedVec |= (y & 0xffL);
	return packedVec;
}

static Vector2i FromInt64(int64_t src)
{
	int y = (int)src;
	int x = (int)(src >> 32);
	return { x, y };
}

Vector2i Vec2iAdd(Vector2i v0, Vector2i v1)
{
	return { v0.x + v1.x, v0.y + v1.y };
}

bool Vector2iu::AreEquals(const Vector2iu o) const
{
	return this->x == o.x && this->y == o.y;
}

uint64_t Vector2iu::ToUInt64() const
{
	uint64_t res = ((uint64_t)x & 0xffULL) << 32;
	res |= (uint64_t)y & 0xffULL;
	return res;
}

Vector2iu Vector2iu::FromUInt64(uint64_t src)
{
	uint32_t y = (uint32_t)src;
	uint32_t x = (uint32_t)(src >> 32);
	return { x, y };
}
