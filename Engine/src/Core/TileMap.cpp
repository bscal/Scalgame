#include "TileMap.h"

#include "Game.h"
#include "SMemory.h"
#include "raymath.h"

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
	outTileMap->MapTiles = 
		(Tile*)Scal::Memory::Alloc(width * height * sizeof(Tile));

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
	outTileSet->TileTypes = 
		(TileType*)Scal::Memory::Alloc(totalTiles * sizeof(TileType));

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

			TileType tileType{};
			tileType.TextureSrcRectangle = rect;
			tileType.MovementCost = 1;

			outTileSet->TileTypes[i] = tileType;
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

			Tile tile{};
			tile.TileId = (uint32_t)tileId;
			tile.Fow = FOWLevel::NoVision;
			tileMap->MapTiles[index] = tile;
		}
	}
}

void UnloadTileMap(TileMap* tileMap)
{
	MemFree(tileMap->MapTiles);
	MemFree(tileMap);
}

global_var Tile* OutTiles[5 * 5 * 5];
global_var bool DisableFow = false;

void RenderTileMap(Game* game, TileMap* tileMap)
{
	const float fowValues[5] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
	Vector4 colorFullVision = ColorNormalize(WHITE);
	Vector4 colorNoVision = ColorNormalize(DARKGRAY);

	Texture2D mapTexture = tileMap->TileSet->TileTexture;

	Player p = game->Player;
	GetSurroundingTiles(
		tileMap,
		p.TilePosition.x,
		p.TilePosition.y,
		5,
		5,
		OutTiles);

	for (int i = 0; i < 5 * 5 * 5; ++i)
	{
		Tile* tile = OutTiles[i];
		if (tile == nullptr) continue;
		tile->Fow = FOWLevel::FullVision;
	}

	if (IsKeyPressed(KEY_F1))
		DisableFow = !DisableFow;

	for (int y = 0; y < tileMap->MapHeight; ++y)
	{
		for (int x = 0; x < tileMap->MapWidth; ++x)
		{
			int index = x + y * tileMap->MapWidth;
			float xPos = (float)x * (float)tileMap->MapTileSize;
			float yPos = (float)y * (float)tileMap->MapTileSize;
			Tile tile = tileMap->MapTiles[index];
			uint32_t tileId = tile.TileId;
			Rectangle textRect =
				tileMap->TileSet->TileTypes[tileId].TextureSrcRectangle;
			Vector2 pos = { xPos, yPos };

			float fowValue = fowValues[(uint8_t)tile.Fow];
			Vector4 finalColor;
			finalColor.x = Lerp(colorNoVision.x, colorFullVision.x, fowValue);
			finalColor.y = Lerp(colorNoVision.y, colorFullVision.y, fowValue);
			finalColor.z = Lerp(colorNoVision.z, colorFullVision.z, fowValue);
			finalColor.w = 1.0f;

			Color color = ColorFromNormalized(finalColor);
			#if SCAL_DEBUG
				if (DisableFow) color = WHITE;
			#endif

			DrawTextureRec(mapTexture, textRect, pos, color);
			tileMap->MapTiles[index].Fow = FOWLevel::NoVision;
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

void GetSurroundingTiles(TileMap* tileMap, int x, int y, int boxWidth, int boxHeight,
	Tile* outTiles[])
{
	int startX = x - boxWidth;
	int startY = y - boxHeight;
	int endX = x + boxWidth;
	int endY = y + boxHeight;
	int i = 0;
	for (int yi = startY; yi <= endY; ++yi)
	{
		for (int xi = startX; xi <= endX; ++xi)
		{
			int index = xi + yi * tileMap->MapWidth;
			if (IsInBounds(xi, yi, tileMap->MapWidth, tileMap->MapHeight))
				outTiles[i] = &tileMap->MapTiles[index];
			else
			{
				outTiles[i] = nullptr;
			}
			++i;
		}
	}
}

TileType* GetTileInfo(TileMap* tileMap, uint32_t tileId)
{
	assert(tileId < 
		tileMap->TileSet->TextureTileWidth * tileMap->TileSet->TextureTileHeight);
	return &tileMap->TileSet->TileTypes[tileId];
}
