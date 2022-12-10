#pragma once

#include "Vector2i.h"
#include "ChunkedTileMap.h"
#include "LightSource.h"

struct World;

void GetTilesInCone(World* world, float playerAngleRadians,
	float coneFovRadians, int rayResolution,
	float x, float y, float distance);

void Raytrace2DInt(World* world, int x0, int y0, int x1, int y1,
	bool (OnVisit)(World* world, int x, int y));

void GetSurroundingTilesRadius(World* world,
	const LightSource& light,
	void (OnVisit)(World* world, int x, int y,
		const LightSource& light));