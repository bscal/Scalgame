#pragma once

#include "Vector2i.h"
#include "ChunkedTileMap.h"

typedef ChunkedTileMap::ChunkedTileMap STileMap;

struct World;

bool IsInBounds(Vector2i startPos, Vector2i endPos,
	Vector2i current);

void GetTilesInCone(World* world, float playerAngleRadians,
	float coneFovRadians, int rayResolution,
	float x, float y, float distance);

void Raytrace2DInt(World* world, int x0, int y0, int x1, int y1,
	bool (OnVisit)(World* world, int x, int y));
