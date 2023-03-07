#pragma once

#include "Core.h"
#include "Globals.h"
#include "Vector2i.h"

#include "Structures/StaticArray.h"

struct Game;

struct LightInfo
{
	float x;
	float y;
	float z;
	float w;

	inline void AssignFromVec4(const Vector4& vec) noexcept { x = vec.x; y = vec.y; z = vec.z; w = vec.w; }
	inline Vector4 AsVec4() const noexcept { return { x, y, z, w }; }
};

struct LightData
{
	Vector2i LightMapOffset;
	StaticArray<LightInfo, SCREEN_TOTAL_TILES> LightColors;
};

void LightMapInitialize(LightData* lightData);

void LightMapUpdate(LightData* lightData, Game* game);

void LightMapSetColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors);

void LightMapSetCeiling(LightData* lightData, TileCoord tileCoord, bool hasCeiling);

bool LightMapInView(LightData* lightData, TileCoord tileCoord);
