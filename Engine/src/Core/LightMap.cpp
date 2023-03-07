#include "LightMap.h"

#include "Game.h"
#include "Lighting.h"
#include "ChunkedTileMap.h"
#include "Renderer.h"

#define LIGHTINFO_DEFAULT LightInfo{ 0.0f, 0.0f, 0.0f, 0.0f }

void LightMapInitialize(LightData* lightData)
{
	lightData->LightColors.fill(LIGHTINFO_DEFAULT);
}

void LightMapUpdate(LightData* lightData, Game* game)
{
	PROFILE_BEGIN();
	lightData->LightMapOffset.x = GetGame()->CullingRect.x / TILE_SIZE_F;
	lightData->LightMapOffset.y = GetGame()->CullingRect.y / TILE_SIZE_F;
	for (int y = 0; y < SCREEN_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH_TILES; ++x)
		{
			int index = x + y * SCREEN_WIDTH_TILES;
			Rectangle rec =
			{
				((float)x + (float)lightData->LightMapOffset.x) * TILE_SIZE_F,
				((float)y + (float)lightData->LightMapOffset.y) * TILE_SIZE_F,
				TILE_SIZE_F,
				TILE_SIZE_F
			};
			SDrawRectangleProF(rec, { 0 }, 0.0f, lightData->LightColors[index].AsVec4());
			lightData->LightColors[index] = LIGHTINFO_DEFAULT; // resets dynamic lights
		}
	}
	PROFILE_END();
}

void LightMapSetColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors)
{
	TileCoord worldTileCoord =
	{
		tileCoord.x - lightData->LightMapOffset.x,
		tileCoord.y - lightData->LightMapOffset.y
	};
	int index = worldTileCoord.x + worldTileCoord.y * SCREEN_WIDTH_TILES;
	if (index < 0 || index >= SCREEN_TOTAL_TILES) return;
	lightData->LightColors[index].AssignFromVec4(colors);
}

void LightMapSetCeiling(LightData* lightData, TileCoord tileCoord, bool HasCieling)
{
	if (!CTileMap::IsTileInBounds(&GetGame()->World.ChunkedTileMap, tileCoord)) return;
	if (!LightMapInView(lightData, tileCoord)) return;

	TileCoord worldTileCoord =
	{
		tileCoord.x - lightData->LightMapOffset.x,
		tileCoord.y - lightData->LightMapOffset.y
	};
	int index = worldTileCoord.x + worldTileCoord.y * SCREEN_WIDTH_TILES;
	lightData->LightColors[index].w = (HasCieling) ? 0.f : 1.f;
}

bool LightMapInView(LightData* lightData, TileCoord tileCoord)
{
	return (tileCoord.x >= GetGame()->LightMap.LightMapOffset.x &&
		tileCoord.y >= GetGame()->LightMap.LightMapOffset.y &&
		tileCoord.x < GetGame()->LightMap.LightMapOffset.x + SCREEN_WIDTH_TILES &&
		tileCoord.y < GetGame()->LightMap.LightMapOffset.y + SCREEN_HEIGHT_TILES);
}