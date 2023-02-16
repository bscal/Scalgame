#pragma once

#include "Core.h"
#include "Vector2i.h"

struct Game;

global_var constexpr int SCREEN_WIDTH = 1600;
global_var constexpr int SCREEN_HEIGHT = 900;
global_var constexpr int TILES_IN_VIEW_PADDING = 2;
global_var constexpr int SCREEN_WIDTH_TILES = (SCREEN_WIDTH / 16) + TILES_IN_VIEW_PADDING;
global_var constexpr int SCREEN_HEIGHT_TILES = (SCREEN_HEIGHT / 16) + TILES_IN_VIEW_PADDING;
global_var constexpr int TILES_IN_VIEW = SCREEN_WIDTH_TILES * SCREEN_HEIGHT_TILES;

struct LightInfo
{
	float x;
	float y;
	float z;
	float w;
	int Count;

	inline void AssignFromVec4(const Vector4& vec) noexcept { x = vec.x; y = vec.y; z = vec.z; w = vec.w; }
	inline Vector4 AsVec4() const noexcept { return { x, y, z, w }; }
};

struct LightData
{
	Vector2i LightMapOffset;
	LightInfo LightColors[TILES_IN_VIEW];
	RenderTexture2D LightMap;
};

void LightMapInitialize(LightData* lightData);

void LightMapUpdate(LightData* lightData, Game* game);

void LightMapSetColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors);

void LightMapAddColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors);
