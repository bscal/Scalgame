#include "SUtil.h"

#include "World.h"

#include <ctype.h> // isspace

Vector4 Vec4Add(const Vector4& v0, const Vector4& v1)
{
	Vector4 v;
	v.x = v0.x + v1.x;
	v.y = v0.y + v1.y;
	v.z = v0.z + v1.z;
	v.w = v0.w + v1.w;
	return v;
}

int IModNegative(int a, int b)
{
	int res = a % b;
	return (res < 0) ? res + b : res;
}

// Temporary list of tile coordinates
SList<Vector2i>
QueryTilesRect(World* world, Vector2i start, Vector2i end)
{
	SASSERT(start.x < end.x);
	SASSERT(start.y < end.y);

	uint32_t width = end.x - start.x;
	uint32_t size = width * (end.y - start.y);
	SList<Vector2i> positions = {};
	positions.Allocator = SAllocator::Temp;
	positions.Reserve(size);

	for (int y = start.y; y < end.y; ++y)
	{
		for (int x = start.x; x < end.x; ++x)
		{
			Vector2i coord = { x, y };
			if (!WorldIsInBounds(world, coord)) continue;
			positions.Push(&coord);
		}
	}
	return positions;
}

// Returns a temporary array of tile positions
SList<Vector2i>
QueryTilesRadius(World* world, Vector2i center, float radius)
{
	SASSERT(radius > 0.0f);

	int startX = center.x - (int)radius;
	int startY = center.y - (int)radius;
	int endX = center.x + (int)radius;
	int endY = center.y + (int)radius;
	float sqrRadius = radius * radius;

	uint32_t size = (endX - startX) + (endY - startY);
	SList<Vector2i> positions = {};
	positions.Allocator = SAllocator::Temp;
	positions.Reserve(size);

	for (int y = startY; y < endY; ++y)
	{
		for (int x = startX; x < endX; ++x)
		{
			Vector2i coord = { x , y };
			if (!WorldIsInBounds(world, coord)) continue;

			float sqrDist = coord.SqrDistance(center);
			if (sqrDist < sqrRadius)
				positions.Push(&coord);
		}
	}
	return positions;
}

Color IntToColor(int colorInt)
{
	Color c = {};
	c.r |= colorInt >> 24;
	c.g |= colorInt >> 16;
	c.b |= colorInt >> 8;
	c.a |= colorInt;
	return c;
}

void TextSplitBuffered(const char* text, char delimiter, int* count, char* buffer, int bufferLength, char** splitBuffer, int splitBufferLength)
{
	splitBuffer[0] = buffer;
	int counter = 0;

	if (text != NULL)
	{
		counter = 1;

		// Count how many substrings we have on text and point to every one
		for (int i = 0; i < bufferLength; i++)
		{
			buffer[i] = text[i];
			if (buffer[i] == '\0') break;
			else if (buffer[i] == delimiter)
			{
				buffer[i] = '\0';   // Set an end of string at this point
				splitBuffer[counter] = buffer + i + 1;
				counter++;

				if (counter == splitBufferLength) break;
			}
		}
	}

	*count = counter;
}

STR2INT Str2Int(int* out, const char* s, int base)
{
	char* end;
	if (s[0] == '\0' || isspace(s[0]))
		return STR2INT_INCONVERTIBLE;
	errno = 0;
	long l = strtol(s, &end, base);
	/* Both checks are needed because INT_MAX == LONG_MAX is possible. */
	if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
		return STR2INT_OVERFLOW;
	if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
		return STR2INT_UNDERFLOW;
	if (*end != '\0')
		return STR2INT_INCONVERTIBLE;
	*out = l;
	return STR2INT_SUCCESS;
}

STR2INT Str2UInt(uint32_t* out, const char* s, int base)
{
	char* end;
#if SCAL_DEBUG
	if (s[0] == '\0' || isspace(s[0]))
		return STR2INT_INCONVERTIBLE;
	errno = 0;
#endif
	uint32_t l = strtoul(s, &end, base);
#if SCAL_DEBUG
	if (*end != '\0')
		return STR2INT_INCONVERTIBLE;
#endif
	*out = l;
	return STR2INT_SUCCESS;
}


int FastAtoi(const char* str)
{
	int val = 0;
	while (*str) {
		val = val * 10 + (*str++ - '0');
	}
	return val;
}
