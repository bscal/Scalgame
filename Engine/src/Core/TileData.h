#pragma once

#include "Core.h"

enum class TileType
{
	Solid,
	Empty,
	Floor
};

enum class TileLOS
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


