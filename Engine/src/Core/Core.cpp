#include "Core.h"

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

Vector2i Vec2iFromInt64(int64_t packedVec2i)
{
	int y = (int)packedVec2i;
	int x = (int)(packedVec2i >> 32);
	return { x, y };
}

Vector2i Vec2iAdd(Vector2i v0, Vector2i v1)
{
	return { v0.x + v1.x, v0.y + v1.y };
}