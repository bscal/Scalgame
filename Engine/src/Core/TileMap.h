#pragma once

#include <Engine.h>

struct Game;

// TODO Think about moving TileType into Tile structure

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

enum class FOWLevel : uint8_t
{
	NoVision = 0,
	PastVision,
	PeripheralVision,
	SemiVision,
	FullVision
};

struct Tile
{
	uint32_t TileId;
	FOWLevel Fow;
};


#define NULL_TILE { 999999999, FOWLevel::NoVision } 

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

// TODO whenever i add dynamic arrays should use those

void GetSurroundingTilesBox(TileMap* tileMap,
	int x, int y,
	int boxWidth, int boxHeight,
	Tile* outTiles[]);

void GetSurroundingTilesRadius(TileMap* tileMap,
	int x, int y,
	float radius,
	Tile** outTiles);

float Distance(float x0, float y0, float x1, float y1);

TileType* GetTileInfo(TileMap* tileMap, uint32_t tileId);
