#include "LightMap.h"

#include "Game.h"
#include "Lighting.h"
#include "ChunkedTileMap.h"
#include "RenderExtensions.h"

void LightMapInitialize(LightData* lightData)
{
	lightData->LightMap = SLoadRenderTexture(SCREEN_WIDTH_TILES, SCREEN_HEIGHT_TILES, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
}

void LightMapUpdate(LightData* lightData, Game* game)
{
	Vector2i target = GetClientPlayer()->Transform.TilePos;
	float offsetX = game->WorldCamera.offset.x / TILE_SIZE + 1.0f;
	float offsetY = game->WorldCamera.offset.y / TILE_SIZE + 1.0f;
	lightData->LightMapOffset.x = (target.x - (int)offsetX);
	lightData->LightMapOffset.y = (target.y - (int)offsetY);

	BeginTextureMode(lightData->LightMap);
	//BeginMode2D(game->WorldCamera);
	ClearBackground({ 0 });

	for (int y = 0; y < SCREEN_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH_TILES; ++x)
		{
			int index = x + y * SCREEN_WIDTH_TILES;
			Rectangle rec = { (float)x, (float)y, 1.0f, 1.0f };
			if (lightData->LightColors[index].Count != 0)
			{
				lightData->LightColors[index].x /= lightData->LightColors[index].Count;
				lightData->LightColors[index].y /= lightData->LightColors[index].Count;
				lightData->LightColors[index].z /= lightData->LightColors[index].Count;
			}
			SDrawRectangleProF(rec, { 0 }, 0.0f, lightData->LightColors[index].AsVec4());
			lightData->LightColors[index] = { 0.f, 0.f, 0.f, 0.f, 0 };
		}
	}

	//EndMode2D();
	EndTextureMode();
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
