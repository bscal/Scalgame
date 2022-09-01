#include "TileMap.h"

#include "Game.h"

#include <cassert>

bool InitializeTileMap(TileSet* tileSet,
	uint32_t width, uint32_t height,
	uint16_t tileSize, TileMap* outTileMap)
{
	outTileMap->TileSet = tileSet;
	outTileMap->MapWidth = width;
	outTileMap->MapHeight = height;
	outTileMap->MapTileSize = tileSize;
	outTileMap->MapHalfTileSize = (float)tileSize / 2.0f;
	outTileMap->MapTiles = (Tile*)MemAlloc(width * height * sizeof(Tile));

	TraceLog(LOG_INFO, "Initialized TileMap");

	return true;
}

bool LoadTileSet(Texture2D* tileTexture,
	uint16_t tileSizeWidth, uint16_t tileSizeHeight,
	TileSet* outTileSet)
{
	outTileSet->TileTexture = *tileTexture;
	outTileSet->TextureTileWidth = tileSizeWidth;
	outTileSet->TextureTileHeight = tileSizeHeight;

	int width = tileTexture->width / tileSizeWidth;
	int height = tileTexture->height / tileSizeHeight;
	int totalTiles = width * height;
	outTileSet->TileTypes = (TileType*)MemAlloc(totalTiles * sizeof(TileType));

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int i = x + y * width;
			float xTexturePos = (float)x * (float)tileSizeWidth;
			float yTexturePos = (float)y * (float)tileSizeHeight;
			float rectX = xTexturePos;
			float rectY = yTexturePos;
			float rectW = tileSizeWidth;
			float rectH = tileSizeHeight;
			Rectangle rect = { rectX, rectY, rectW, rectH };
			outTileSet->TileTypes[i].TextureSrcRectangle = rect;
			outTileSet->TileTypes[i].MovementCost = 1;
		}
	}
	
	TraceLog(LOG_INFO, "Loaded TileSet %d with %d tiles",
		tileTexture->id, totalTiles);

	return true;
}

void LoadTileMap(TileMap* tileMap)
{
	SetRandomSeed(0);
	for (int y = 0; y < tileMap->MapHeight; ++y)
	{
		for (int x = 0; x < tileMap->MapWidth; ++x)
		{
			int index = x + y * tileMap->MapWidth;
			int tileId = GetRandomValue(0, 2);
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
	Texture2D mapTexture = tileMap->TileSet->TileTexture;

	for (int y = 0; y < tileMap->MapHeight; ++y)
	{
		for (int x = 0; x < tileMap->MapWidth; ++x)
		{
			int index = x + y * tileMap->MapWidth;
			float xPos = (float)x * (float)tileMap->MapTileSize;
			float yPos = (float)y * (float)tileMap->MapTileSize;
			uint32_t tileId = tileMap->MapTiles[index].TileId;
			Rectangle textRect =
				tileMap->TileSet->TileTypes[tileId].TextureSrcRectangle;
			Vector2 pos = { xPos, yPos };
			DrawTextureRec(mapTexture, textRect, pos, WHITE);
		}
	}

	DrawRectangleLinesEx(
		{0, 0,
		(float)tileMap->MapWidth * (float)tileMap->MapTileSize,
		(float)tileMap->MapHeight * (float)tileMap->MapTileSize
		},
		2.5f,
		RED);
}

bool IsInBounds(int x, int y, int width, int height)
{
	return x >= 0 && x < width && y >= 0 && y < height;
}

Tile* GetTile(TileMap* tileMap, int x, int y)
{
	assert(x >= 0 && x < tileMap->MapWidth);
	assert(y >= 0 && y < tileMap->MapHeight);

	int index = x + y * tileMap->MapWidth;
	return &tileMap->MapTiles[index];
}

void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile)
{
	assert(x >= 0 && x < tileMap->MapWidth);
	assert(y >= 0 && y < tileMap->MapHeight);

	int index = x + y * tileMap->MapWidth;
	tileMap->MapTiles[index] = *srcTile;
}

TileType* GetTileInfo(TileMap* tileMap, uint32_t tileId)
{
	assert(tileId < 
		tileMap->TileSet->TextureTileWidth * tileMap->TileSet->TextureTileHeight);
	return &tileMap->TileSet->TileTypes[tileId];
}
