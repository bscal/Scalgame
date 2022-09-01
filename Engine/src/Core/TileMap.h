#pragma once

#include <Engine.h>

struct Game;

struct TileType
{
	Rectangle TextureSrcRectangle;
	uint16_t MovementCost;
};

struct TileSet
{
	// TODO should a TileSet have a unique Texture?
	Texture2D TileTexture;
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
	TileSet* TileSet;
	Tile* MapTiles;
	uint32_t MapWidth;
	uint32_t MapHeight;
	uint16_t MapTileSize;
	float MapHalfTileSize;
};

bool LoadTileSet(Texture2D* tileTexture,
	uint16_t tileSizeWidth, uint16_t tileSizeHeight,
	TileSet* outTileSet);

bool InitializeTileMap(TileSet* tileSet, 
	uint32_t width, uint32_t height,
	uint16_t tileSize, TileMap* outTileMap);

void LoadTileMap(TileMap* tileMap);

void UnloadTileMap(TileMap* tileMap);
void RenderTileMap(Game* game, TileMap* tileMap);

bool IsInBounds(int x, int y, int width, int height);

Tile* GetTile(TileMap* tileMap, int x, int y);
void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile);

TileType* GetTileInfo(TileMap* tileMap, uint32_t tileId);
