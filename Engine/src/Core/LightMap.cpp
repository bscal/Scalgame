#include "LightMap.h"

#include "Game.h"
#include "Lighting.h"
#include "ChunkedTileMap.h"
#include "Renderer.h"

#define LIGHTINFO_DEFAULT LightInfo{  0.0f, 0.0f, 0.0f, 0.0f, 0 }

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
			if (lightData->LightColors[index].Count > 0)
			{
				Vector4 color = lightData->LightColors[index].AsVec4();
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
	lightData->LightColors[index].Count = 1;
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
	SASSERT(index < TILES_IN_VIEW)
	lightData->LightColors[index].x += colors.x;
	lightData->LightColors[index].y += colors.y;
	lightData->LightColors[index].z += colors.z;
	lightData->LightColors[index].w += colors.w;
	++lightData->LightColors[index].Count;
}
