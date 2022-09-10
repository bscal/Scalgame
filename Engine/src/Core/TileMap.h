#pragma once

#include <Engine.h>

struct Game;

// TODO Think about moving TileType into Tile structure

enum class TileVisibilty : uint8_t
{
	Empty = 0,
	Solid
};

enum class TileType : uint8_t
{
	Empty = 0,
	Solid,
	Floor,
	Liquid
};

struct TileData
{
	Rectangle TextureSrcRectangle;
	short MovementCost;
	TileVisibilty TileVisibilty;
	TileType TileType;
};

struct TileSet
{
	Texture2D TileTexture;
	TileData* TileDataArray;
	uint32_t TextureTileWidth;
	uint32_t TextureTileHeight;
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
	uint64_t MapSize;
	uint32_t MapWidth;
	uint32_t MapHeight;
	uint16_t MapTileSize;
	uint16_t MapHalfTileSize;
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

bool IsInBounds(int tileX, int tileY, int width, int height);

Tile* GetTile(TileMap* tileMap, int tileX, int tileY);
void SetTile(TileMap* tileMap, int tileX, int tileY, Tile* srcTile);
TileData* GetTileData(TileMap* tileMap, uint32_t tileId);

// TODO whenever i add dynamic arrays should use those

void GetSurroundingTilesBox(TileMap* tileMap,
	int x, int y,
	int boxWidth, int boxHeight,
	Tile** outTiles);

void GetSurroundingTilesRadius(TileMap* tileMap,
	int x, int y,
	float radius,
	Tile** outTiles);

void GetSurronding(TileMap* tileMap, float x, float y,
	int resolution, float distance);

void GetTilesInCone(TileMap* tileMap,
	float playerAngle, float playerFov,
	float x, float y, float distance);

float Distance(float x0, float y0, float x1, float y1);

TileData* GetTileInfo(TileMap* tileMap, uint32_t tileId);
