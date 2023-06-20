#pragma once

#include "Core.h"

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

void ProcessLightUpdater(UpdatingLight* light, uint32_t screenLightsWidth, Vector3* colorsArray, ChunkedTileMap* tilemap);

void ThreadedLightUpdate(Light* light, Vector3* threadColorsArray, ChunkedTileMap* tilemap, uint32_t LightsScreenWidth);
