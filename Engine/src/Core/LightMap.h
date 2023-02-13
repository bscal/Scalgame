#pragma once

#include "Core.h"
#include "Lighting.h"
#include "ChunkedTileMap.h"

struct LightData
{
	Vector3 LightColors[CHUNK_SIZE];
	Color StaticColors[CHUNK_SIZE];
};

void SetColor(Vector3 colors);
void SetDarkness(uint8_t opacity);

void AddLight(const Light* light);
void RemoveLight(const Light* light);

struct LightMap
{
	
};
