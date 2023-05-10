#pragma once

#include "Core.h"
#include "Globals.h"

struct UpdatingLight;
struct Vector3;
struct ChunkedTileMap;

struct ThreadedLights
{
	Vector3* ColorArrayPtrs[4];
	int Count = 4;

	void AllocateArrays(size_t elementCount);
	void UpdateLightColorArray(Vector4* finalColors) const;
};

void ProcessLightUpdater(UpdatingLight* light, size_t width, Vector3* colorsArray, ChunkedTileMap* tilemap);
