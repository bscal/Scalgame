#pragma once

#include <Engine.h>

struct Game;

struct TileType
{
	Rectangle TextureSrcRectangle;
	uint16_t MovementCost;
};

struct TextureTileSet
{
	Texture2D TileTextures;
	TileType* TileTypes;
	uint16_t TextureTileWidth;
	uint16_t TextureTileHeight;
};

struct Tile
{
	uint32_t TileId;
};

struct TileMap
{
	TextureTileSet* TileSetPtr;
	Tile* MapTiles;
	uint32_t MapWidth;
	uint32_t MapHeight;
	uint16_t MapTileSize;
	float MapHalfTileSize;
};

bool LoadTileSet(const char* textureFilePath,
	uint16_t tileSizeWidth, uint16_t tileSizeHeight,
	TextureTileSet* outTileSet);

bool InitializeTileMap(TextureTileSet* tileSet, 
	uint32_t width, uint32_t height,
	uint16_t tileSize, TileMap* outTileMap);

void LoadTileMap(TileMap* tileMap);

void UnloadTileMap(TileMap* tileMap);
void RenderTileMap(Game* game, TileMap* tileMap);

Tile* GetTile(TileMap* tileMap, int x, int y);
void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile);
