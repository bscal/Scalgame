#include "TileMapUtils.h"

#include "World.h"
#include "ChunkedTileMap.h"

#include "raymath.h"

bool IsInBounds(Vector2i startPos, Vector2i endPos,
	Vector2i current)
{
	return (current.x >= startPos.x &&
		current.y >= startPos.y &&
		current.x <= endPos.x &&
		current.y <= endPos.y);
}

internal bool OnVisitTile(World* world, int x, int y)
{
	Vector2i pos = { x, y };
	if (IsInWorldBounds(world, pos) &&
		world->SightMap.IsInBounds(pos))
	{
		//world->SightMap.AddSight(pos, 1.0f);
		//const auto tile = ChunkedTileMap::GetTile(
		//	&world->ChunkedTileMap, x, y);
		//return tile->IsSolid;
	}
	return false;
}

void GetTilesInCone(World* world, float playerAngleRadians,
	float coneFovRadians, int rayResolution,
	float x, float y, float distance)
{
	coneFovRadians /= 2;
	float startAngle = 0.0f;
	float endAngle = coneFovRadians;
	for (int i = 0; i < rayResolution; ++i)
	{
		float t = Lerp(startAngle, endAngle,
			(float)i / (float)rayResolution);
		float x0 = cosf(playerAngleRadians + t);
		float y0 = sinf(playerAngleRadians + t);
		float x1 = cosf(playerAngleRadians - t);
		float y1 = sinf(playerAngleRadians - t);

		float xPos0 = x + x0 * distance;
		float yPos0 = y + y0 * distance;
		float xPos1 = x + x1 * distance;
		float yPos1 = y + y1 * distance;

		Raytrace2DInt(world, (int)x, (int)y,
			(int)xPos0, (int)yPos0, OnVisitTile);
		Raytrace2DInt(world, (int)x, (int)y,
			(int)xPos1, (int)yPos1, OnVisitTile);

		DrawLineEx({ x, y }, { xPos0, yPos0 }, 2.0f, PINK);
		DrawLineEx({ x, y }, { xPos1, yPos1 }, 2.0f, PINK);
	}
}

void Raytrace2DInt(World* world, int x0, int y0, int x1, int y1,
	bool (OnVisit)(World* world, int x, int y))
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
		if (OnVisit(world, x, y))
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
