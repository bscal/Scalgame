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
	Vector2i target = GetClientPlayer()->Transform.TilePos;
	const float lPadding = 2.0f;
	float offsetX = game->WorldCamera.offset.x / TILE_SIZE_F + lPadding;
	float offsetY = game->WorldCamera.offset.y / TILE_SIZE_F + lPadding;
	lightData->LightMapOffset.x = (target.x - (int)offsetX);
	lightData->LightMapOffset.y = (target.y - (int)offsetY);
	for (int y = 0; y < SCREEN_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH_TILES; ++x)
		{
			int index = x + y * SCREEN_WIDTH_TILES;
			SASSERT(index >= 0);
			SASSERT(index < SCREEN_TOTAL_TILES);
			const Vector4& color = lightData->LightColors[index].AsVec4();
			if (color.x == 0.f && color.y == 0.f && color.z == 0.f && color.w == 0.f) continue;
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

void LightMapAddColor(LightData* lightData, TileCoord tileCoord, const Vector4& colors)
{
	TileCoord worldTileCoord =
	{
		tileCoord.x - lightData->LightMapOffset.x,
		tileCoord.y - lightData->LightMapOffset.y
	};
	int index = worldTileCoord.x + worldTileCoord.y * SCREEN_WIDTH_TILES;

	SASSERT(index >= 0);
	SASSERT(index < SCREEN_TOTAL_TILES);
	lightData->LightColors[index].x += colors.x;
	lightData->LightColors[index].y += colors.y;
	lightData->LightColors[index].z += colors.z;
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