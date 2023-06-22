#pragma once

#include "Core.h"

struct Light;
struct UpdatingLight;
struct Vector3;
struct ChunkedTileMap;

#define LIGHT_MAX_THEADS 8

struct ThreadedLights
{
	Vector3* ColorArrayPtrs[LIGHT_MAX_THEADS];

	void AllocateArrays(size_t elementCount, int numOfArrays);
	void UpdateLightColorArray(Vector4* finalColors, int numOfArrays) const;
};

void 
ProcessLightUpdater(UpdatingLight* light, uint32_t screenLightsWidth, Vector3* colorsArray, ChunkedTileMap* tilemap);

void 
ThreadedLightUpdate(Light* light, Vector3* threadColorsArray, ChunkedTileMap* tilemap, uint32_t LightsScreenWidth);
