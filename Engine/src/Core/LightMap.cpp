#include "LightMap.h"

#include "Game.h"
#include "Lighting.h"
#include "ChunkedTileMap.h"
#include "Renderer.h"

#define LIGHTINFO_DEFAULT LightInfo{  0.0f, 0.0f, 0.0f, 0.0f }

void LightMapInitialize(LightData* lightData)
{
	for (int i = 0; i < TILES_IN_VIEW; ++i)
	{
		lightData->LightColors[i] = LIGHTINFO_DEFAULT;
	}
}

void LightMapUpdate(LightData* lightData, Game* game)
{
	Vector2i target = GetClientPlayer()->Transform.TilePos;
	const float lPadding = 1.0f;
	float offsetX = game->WorldCamera.offset.x / TILE_SIZE_F - lPadding;
	float offsetY = game->WorldCamera.offset.y / TILE_SIZE_F - lPadding;
	lightData->LightMapOffset.x = (target.x - (int)offsetX);
	lightData->LightMapOffset.y = (target.y - (int)offsetY);

	BeginBlendMode(BLEND_ADDITIVE);
	for (int y = 0; y < SCREEN_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH_TILES; ++x)
		{
			int index = x + y * SCREEN_WIDTH_TILES;
			const Vector4& color = lightData->LightColors[index].AsVec4();
			Rectangle rec =
			{
				((float)x + (float)lightData->LightMapOffset.x) * TILE_SIZE_F,
				((float)y + (float)lightData->LightMapOffset.y) * TILE_SIZE_F,
				TILE_SIZE_F,
				TILE_SIZE_F
			};
			SDrawRectangleProF(rec, { 0 }, 0.0f, color);
			lightData->LightColors[index] = LIGHTINFO_DEFAULT; // resets dynamic lights
		}
	}
	EndBlendMode();
}

void LightMapSetColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors)
{
	TileCoord worldTileCoord =
	{
		tileCoord.x - lightData->LightMapOffset.x,
		tileCoord.y - lightData->LightMapOffset.y
	};
	int index = worldTileCoord.x + worldTileCoord.y * SCREEN_WIDTH_TILES;
	if (index < 0 || index >= TILES_IN_VIEW) return;
	lightData->LightColors[index].AssignFromVec4(colors);
}

void LightMapAddColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors)
{
	TileCoord worldTileCoord =
	{
		tileCoord.x - lightData->LightMapOffset.x,
		tileCoord.y - lightData->LightMapOffset.y
	};
	int index = worldTileCoord.x + worldTileCoord.y * SCREEN_WIDTH_TILES;
	SASSERT(index >= 0);
	SASSERT(index < TILES_IN_VIEW);
	lightData->LightColors[index].x += colors.x;
	lightData->LightColors[index].y += colors.y;
	lightData->LightColors[index].z += colors.z;
}

internal inline bool
IsTileValid(Vector2i pos)
{
	if (!CTileMap::IsTileInBounds(&GetGame()->World.ChunkedTileMap, pos)) return false;
	if (pos.x < GetGame()->LightMap.LightMapOffset.x ||
		pos.y < GetGame()->LightMap.LightMapOffset.y ||
		pos.x >= GetGame()->LightMap.LightMapOffset.x + SCREEN_WIDTH_TILES ||
		pos.y >= GetGame()->LightMap.LightMapOffset.y + SCREEN_HEIGHT_TILES) return false;
	return true;
}

void LightMapSetCeiling(LightData* lightData, TileCoord tileCoord, bool HasCieling)
{
	if (!IsTileValid(tileCoord)) return;
	TileCoord worldTileCoord =
	{
		tileCoord.x - lightData->LightMapOffset.x,
		tileCoord.y - lightData->LightMapOffset.y
	};
	int index = worldTileCoord.x + worldTileCoord.y * SCREEN_WIDTH_TILES;
	lightData->LightColors[index].w = (HasCieling) ? 0.f : 1.f;
}