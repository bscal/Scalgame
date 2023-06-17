#include "Game.h"
#include "World.h"
#include "ChunkedTileMap.h"
#include "Structures/SHashSet.h"

#include "raymath.h"

internal bool OnVisitTile(World* world, int x, int y)
{
	Vector2i pos = { x, y };
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

		//Raytrace2DInt(world, (int)x, (int)y,
			//(int)xPos0, (int)yPos0, OnVisitTile);
		//Raytrace2DInt(world, (int)x, (int)y,
			//(int)xPos1, (int)yPos1, OnVisitTile);

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

internal float Distance(float x0, float y0, float x1, float y1)
{
	float xL = x1 - x0;
	float yL = y1 - y0;
	return sqrtf(xL * xL + yL * yL);
}

//void GetSurroundingTilesRadius(World* world,
//	const LightSource& light,
//	void (OnVisit)(World* world, int x, int y,
//		const LightSource& light))
//{
//	int startX = (int)(light.Position.x - light.Intensity);
//	int startY = (int)(light.Position.y - light.Intensity);
//	int endX = (int)(light.Position.x + light.Intensity);
//	int endY = (int)(light.Position.y + light.Intensity);
//	for (int yi = startY; yi <= endY; ++yi)
//	{
//		for (int xi = startX; xi <= endX; ++xi)
//		{
//			if (WorldIsInBounds(world, { xi, yi }) &&
//				Distance(light.Position.x, light.Position.y,
//					(float)xi + 0.5f, (float)yi + 0.5f) < light.Intensity)
//			{
//				OnVisit(world, xi, yi, light);
//			}
//		}
//	}
//}

void
IsInCone(SList<Vector2i>* tilesInLos, Vector2 line0, Vector2 line1)
{
	ChunkedTileMap* tilemap = &GetGame()->Universe.World.ChunkedTileMap;
	for (uint32_t i = 0; i < tilesInLos->Count; ++i)
	{
		Vector2i tile = tilesInLos->Memory[i];
		float dot0 = Vector2DotProduct(line0, tile.AsVec2());
		float dot1 = Vector2DotProduct(line1, tile.AsVec2());
		if (dot0 > 0 && dot1 < 0)
		{
			CTileMap::SetVisible(tilemap, tile);
		}
	}
}

void GetTilesInLOS(SHashSet<Vector2i>* tiles, Vector2i pos, float distance, float fov)
{
	float playerAngleRadians = AngleFromTileDir(GetClientPlayer()->LookDir);
	float coneFov = (fov * DEG2RAD) / 2.0f;
	float x = (float)pos.x + HALF_TILE_SIZE;
	float y = (float)pos.y + HALF_TILE_SIZE;
	float startAngle = 0.0f;
	float endAngle = coneFov;

	int rayResolution = 32;
	for (int i = 0; i < rayResolution; ++i)
	{
		float t = Lerp(startAngle, endAngle, (float)i / (float)rayResolution);
		float x0 = cosf(playerAngleRadians + t);
		float y0 = sinf(playerAngleRadians + t);
		float x1 = cosf(playerAngleRadians - t);
		float y1 = sinf(playerAngleRadians - t);

		float xPos0 = x + x0 * distance;
		float yPos0 = y + y0 * distance;
		float xPos1 = x + x1 * distance;
		float yPos1 = y + y1 * distance;

		//Raytrace2DInt(tileMap, x, y, xPos0, yPos0, LOSRayHit);
		//Raytrace2DInt(tileMap, x, y, xPos1, yPos1, LOSRayHit);
	}
}

//void Raytrace2D(float x0, float y0, float x1, float y1, bool* values)
//{
//	float dx = fabsf(x1 - x0);
//	float dy = fabsf(y1 - y0);
//	int x = (int)floorf(x0);
//	int y = (int)floorf(y0);
//	int n = 1;
//	int xInc;
//	int yInc;
//	float error;
//	if (dx == 0)
//	{
//		xInc = 0;
//		error = INFINITY;
//	} else if (x1 > x0)
//	{
//		xInc = 1;
//		n += (int)floorf(x1) - x;
//		error = (floorf(x0) + 1 - x0) * dy;
//	} else
//	{
//		xInc = -1;
//		n += x - (int)floorf(x1);
//		error = (x0 - floorf(x0)) * dy;
//	}
//
//	if (dy == 0)
//	{
//		yInc = 0;
//		error -= INFINITY;
//	} else if (y1 > y0)
//	{
//		yInc = 1;
//		n += (int)floorf(y1) - y;
//		error -= (floorf(y0) + 1 - y0) * dx;
//	} else
//	{
//		yInc = -1;
//		n += y - (int)floorf(y1);
//		error -= (y0 - floorf(y0)) * dx;
//	}
//
//	for (; n > 0; --n)
//	{
//		// TODO CHANGE 64
//		if (IsInBounds(x, y, 64, 64))
//			values[x + y * 64] = true;
//
//		if (error > 0.0f)
//		{
//			y += yInc;
//			error -= dx;
//		} else
//		{
//			x += xInc;
//			error += dy;
//		}
//	}
//}