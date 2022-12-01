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
	TileLOS LOS;
	bool IsUndiscovered;
	bool IsOccupied;
};

#define MAX_TILES 255

struct TileMgr
{
	TileData Tiles[MAX_TILES];
	uint32_t NextTileId;
};

bool TileMgrInitialize(TileMgr* tileMgr);
bool TileMgrRegisterTile(TileMgr* tileMgr, TileData* data);

Tile CreateTile(TileMgr* tileMgr, uint32_t tileId);


