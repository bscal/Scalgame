#include "SUtil.h"

#include "World.h"

Vector4 Vec4Add(const Vector4& v0, const Vector4& v1)
{
	Vector4 v;
	v.x = v0.x + v1.x;
	v.y = v0.y + v1.y;
	v.z = v0.z + v1.z;
	v.w = v0.w + v1.w;
	return v;
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
	positions.Allocator = SMEM_TEMP_ALLOCATOR;
	positions.EnsureCapacity(size);

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

	int startX = center.x - radius;
	int startY = center.y - radius;
	int endX = center.x + radius;
	int endY = center.y + radius;
	float sqrRadius = radius * radius;

	uint32_t size = (endX - startX) + (endY - startY);
	SList<Vector2i> positions = {};
	positions.Allocator = SMEM_TEMP_ALLOCATOR;
	positions.EnsureCapacity(size);

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

