#pragma once

#include "Core.h"
#include "SpriteAtlas.h"

#include <string.h>
#include <string_view>

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
	std::string SpriteName;
	uint32_t TileId;
	int MovementCost;
	short Cover;
	TileType Type;
};

#define MAX_TILES 32
struct TileMgr
{
	SpriteAtlas* SpriteAtlas;
	TileData Tiles[MAX_TILES];
	uint32_t NextTileId;
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

	const TileData& GetTileData(TileMgr* tileMgr) const;
};

void TileMgrInitialize(TileMgr* tileMgr, SpriteAtlas* spriteAtlas);

TileData& RegisterTile(TileMgr* tileMgr,
	std::string_view spriteName,
	TileType type);

[[nodiscard]] Tile CreateTile(TileMgr* tileMgr, const TileData& tileData);
[[nodiscard]] Tile CreateTileId(TileMgr* tileMgr, uint32_t tileDataId);

//bool TileMgrInitialize(TileMgr* tileMgr);
//bool TileMgrRegisterTile(TileMgr* tileMgr, TileData* data);
//
//Tile CreateTile(TileMgr* tileMgr, TileId tileId);
//
//void SetTile(TileMgr* tileMgr, Tile* tile, TileId newTileId);
//const TileData& GetTileData(TileMgr* tileMgr, TileId tileId);


