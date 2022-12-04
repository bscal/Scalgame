#pragma once

#include "Core.h"

enum class TileType : uint8_t
{
	Solid,
	Empty,
	Floor
};

enum class TileLOS : uint8_t
{
	NoVision,
	FullVision,
	HalfVision
};

struct TileData
{
	Rectangle TextureRect;
	uint32_t TileId;
	int MovementCost;
	short Cover;
	TileType Type;
};

struct Tile
{
	Rectangle TextureRect;
	Color TileColor;
	uint32_t TileDataId;
	uint8_t LightLevel;
	TileLOS LOS;
	bool IsUndiscovered;
	bool IsOccupied;
};

#define MAX_TILES 255

typedef uint32_t TileId;

struct TileMgr
{
	TileData Tiles[MAX_TILES];
	TileId NextTileId;
};

bool TileMgrInitialize(TileMgr* tileMgr);
bool TileMgrRegisterTile(TileMgr* tileMgr, TileData* data);

Tile CreateTile(TileMgr* tileMgr, TileId tileId);

void SetTile(TileMgr* tileMgr, Tile* tile, TileId newTileId);
const TileData& GetTileData(TileMgr* tileMgr, TileId tileId);


