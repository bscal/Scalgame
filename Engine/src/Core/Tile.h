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
};

struct TileSheetCoord
{
	uint8_t x;
	uint8_t y;
};

#define BLACK_FLOOR	TileSheetCoord{0, 0}
#define STONE_FLOOR	TileSheetCoord{4, 0}
#define GOLD_ORE TileSheetCoord{10, 0}
#define DARK_STONE_FLOOR TileSheetCoord{14, 0}
#define ROCKY_WALL TileSheetCoord{20, 2}

struct Tile
{
	bool IsUsed;
	TileType Type;
};

#define TILE_SHEET_WIDTH 512
#define TILE_SHEET_HEIGHT 960
#define TILE_SHEET_WIDTH_TILES (TILE_SHEET_WIDTH / 16)
#define TILE_SHEET_HEIGHT_TILES (TILE_SHEET_HEIGHT / 16)
struct TileMgr
{
	Texture2D TileTextureRef;
	Tile Tiles[TILE_SHEET_WIDTH_TILES * TILE_SHEET_HEIGHT_TILES];
};

struct TileData
{
	uint8_t TexX;
	uint8_t TexY;
	uint8_t NOT_USED;
	TileLOS LOS;

	inline uint16_t GetTileId() const
	{
		uint16_t id = TexX;
		id |= ((uint16_t)TexY << 8);
		return id;
	};

	Tile* GetTile() const;
};

static_assert(sizeof(TileData) == sizeof(int), "Tile size must be 32 bits");

bool TileMgrInitialize(const Texture2D* tilesheetTexture);

uint16_t TileMgrRegister(TileSheetCoord coord, TileType type);

struct TileMgr* GetTileMgr();

TileData TileMgrCreate(uint16_t tileId);

inline TileSheetCoord TileMgrGetXY(uint16_t tileId)
{
	TileSheetCoord res;
	res.x = (uint8_t)tileId;
	res.y = (uint8_t)(tileId >> 8);
	return res;
}
