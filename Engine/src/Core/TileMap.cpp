#include "TileMap.h"

#include <cassert>

bool InitializeTileMap(
	Texture2D* Texture,
	uint32_t width,
	uint32_t height,
	uint16_t tileSize,
	TileMap* outMap)
{
	TileMap* tileMap = (TileMap*)MemAlloc(sizeof(TileMap));
	tileMap->Texture = Texture;
	tileMap->Width = width;
	tileMap->Height = height;
	tileMap->TileSize = tileSize;
	tileMap->Tiles = (Tile*)MemAlloc(width * height * sizeof(Tile));
	outMap = tileMap;
	return true;
}

void UnloadTileMap(TileMap* tileMap)
{
	MemFree(tileMap->Tiles);
	MemFree(tileMap);
}

void RenderTileMap(TileMap* tileMap)
{
	Texture2D texture = *tileMap->Texture;
	for (int y = 0; y < tileMap->Height; ++y)
	{
		for (int x = 0; x < tileMap->Width; ++x)
		{
			int index = x + y * tileMap->Width;
			uint32_t tileId = tileMap->Tiles[index].TileId;

			float textX = tileId % tileMap->Width;
			float textY = tileId / tileMap->Width;

			float textureTileSize = 16;
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
			DrawTextureRec(texture, textureSrc, tilePos, WHITE);
		}
	}
}

Tile* GetTile(TileMap* tileMap, int x, int y)
{
	assert(x >= 0 && x <= tileMap->Width);
	assert(y >= 0 && y <= tileMap->Height);

	int index = x + y * tileMap->Width;
	return &tileMap->Tiles[index];
}

void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile)
{
	assert(x >= 0 && x <= tileMap->Width);
	assert(y >= 0 && y <= tileMap->Height);

	int index = x + y * tileMap->Width;
	tileMap->Tiles[index] = *srcTile;
}
