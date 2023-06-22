#pragma once

#include "Core.h"
#include "Vector2i.h"

struct TileData;

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
#define LAVA_0 TileSheetCoord{31,54}

struct Tile
{
	bool IsUsed;
	bool EmitsLight;
	TileType Type;

	void(*OnUpdate)(Vector2i, TileData);
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

struct TileTexValues
{
	uint8_t x;
	uint8_t y;
	uint8_t HasCeiling;
	uint8_t LOS;
};

struct TileData
{
	uint8_t TexX;
	uint8_t TexY;
	bool HasCeiling;
	TileLOS LOS;

	inline constexpr TileSheetCoord AsCoord() const { return { TexX, TexY }; }

	Tile* GetTile() const;
};

bool TileMgrInitialize(const Texture2D* tilesheetTexture);

uint16_t TileMgrRegister(TileSheetCoord coord, TileType type);
uint16_t TileMgrRegister(TileSheetCoord coord);

struct TileMgr* GetTileMgr();

TileData TileMgrCreate(uint16_t tileId);

inline constexpr uint16_t TileMgrToTileId(TileSheetCoord coord)
{
	uint16_t id = coord.x;
	id |= ((uint16_t)coord.y << 8);
	return id;
}

inline constexpr TileSheetCoord TileMgrGetXY(uint16_t tileId)
{
	TileSheetCoord res = 
	{
		(uint8_t)tileId,
		(uint8_t)(tileId >> 8)
	};
	return res;
}
