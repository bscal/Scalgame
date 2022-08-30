#include "TileMap.h"

#include "Game.h"

#include <cassert>

bool InitializeTileMap(
	Texture2D* Texture,
	uint32_t width,
	uint32_t height,
	uint16_t tileSize,
	TileMap* outMap)
{
	outMap->Texture = Texture;
	outMap->Width = width;
	outMap->Height = height;
	outMap->TileSize = tileSize;
	outMap->MapTiles = (Tile*)MemAlloc(width * height * sizeof(Tile));
	return true;
}

void LoadTileMap(TileMap* tileMap)
{
	SetRandomSeed(0);
	for (int y = 0; y < tileMap->Height; ++y)
	{
		for (int x = 0; x < tileMap->Width; ++x)
		{
			int index = x + y * tileMap->Width;
			int tileId = GetRandomValue(0, 1);
			tileMap->MapTiles[index].TileId = (uint32_t) tileId;
		}
	}
}

void UnloadTileMap(TileMap* tileMap)
{
	MemFree(tileMap->MapTiles);
	MemFree(tileMap);
}

void RenderTileMap(Game* game, TileMap* tileMap)
{
	Texture2D mapTexture = game->Engine.Resources.MainTileMapTexture;
	for (int y = 0; y < tileMap->Height; ++y)
	{
		for (int x = 0; x < tileMap->Width; ++x)
		{
			int index = x + y * tileMap->Width;
			uint32_t tileId = tileMap->MapTiles[index].TileId;

			float textureTileSize = 16;
			float textX = (tileId % tileMap->Width) * textureTileSize;
			float textY = (tileId / tileMap->Width) * textureTileSize;

			Rectangle textureSrc = {};
			textureSrc.x = textX;
			textureSrc.y = textY;
			textureSrc.width = textX + textureTileSize;
			textureSrc.height = textY + textureTileSize;

			Vector2 tilePos = 
			{ 
				x * tileMap->TileSize,
				y * tileMap->TileSize
			};
			DrawTextureRec(mapTexture, textureSrc, tilePos, WHITE);
		}
	}
}

Tile* GetTile(TileMap* tileMap, int x, int y)
{
	assert(x >= 0 && x <= tileMap->Width);
	assert(y >= 0 && y <= tileMap->Height);

	int index = x + y * tileMap->Width;
	return &tileMap->MapTiles[index];
}

void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile)
{
	assert(x >= 0 && x <= tileMap->Width);
	assert(y >= 0 && y <= tileMap->Height);

	int index = x + y * tileMap->Width;
	tileMap->MapTiles[index] = *srcTile;
}
