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

Vector2i Vec2iAdd(Vector2i v0, Vector2i v1)
{
	return { v0.x + v1.x, v0.y + v1.y };
}