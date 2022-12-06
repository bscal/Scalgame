#pragma once

#include "Vector2i.h"
#include "ChunkedTileMap.h"

typedef ChunkedTileMap::ChunkedTileMap STileMap;

struct World;

//bool IsInBounds(Vector2i startPos, Vector2i endPos, Vector2i current);

void GetTilesInCone(World* world, STileMap* tilemap,
	float playerAngleRadians, float coneFovRadians, int rayResolution,
	float x, float y, float distance);
