#pragma once

#include "Core.h"

struct Light;
struct UpdatingLight;
struct StaticLight;
struct Vector3;
struct ChunkedTileMap;

void 
ProcessLightUpdater(UpdatingLight* light, uint32_t screenLightsWidth, Vector3* colorsArray, ChunkedTileMap* tilemap);

void 
ThreadedLightUpdate(Light* light, Vector3* threadColorsArray, ChunkedTileMap* tilemap, uint32_t LightsScreenWidth);

void
UpdateStaticLight(StaticLight* light, Vector3* threadColorsArray, size_t width);