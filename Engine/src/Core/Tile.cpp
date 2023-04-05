#include "Tile.h"

#include "Game.h"
#include "Lighting.h"

global_var struct TileMgr TileMgr;

bool TileMgrInitialize(const Texture2D* tilesheetTexture)
{
	SASSERT(tilesheetTexture);
	SASSERT(tilesheetTexture->id > 0);

	TileMgr.TileTextureRef = *tilesheetTexture;

	SASSERT(TileMgr.TileTextureRef.width == TILE_SHEET_WIDTH);
	SASSERT(TileMgr.TileTextureRef.height == TILE_SHEET_HEIGHT);

	TileMgrRegister(BLACK_FLOOR, TileType::Solid);
	TileMgrRegister(STONE_FLOOR, TileType::Floor);
	uint16_t gold = TileMgrRegister(GOLD_ORE, TileType::Floor);
	TileMgrRegister(DARK_STONE_FLOOR, TileType::Floor);
	TileMgrRegister(ROCKY_WALL, TileType::Solid);

	uint16_t lava0 = TileMgrRegister(LAVA_0, TileType::Floor);
	TileMgr.Tiles[lava0].OnUpdate = [](Vector2i pos, TileData data)
	{
		DrawStaticLavaLight(pos, RED);
	};

	return true;
}

struct TileMgr* GetTileMgr() { return &TileMgr; }

uint16_t TileMgrRegister(TileSheetCoord coord, TileType type)
{
	uint16_t result = TileMgrRegister(coord);
	TileMgr.Tiles[result].Type = type;
	return result;
}

uint16_t TileMgrRegister(TileSheetCoord coord)
{
	SASSERT(coord.x < TILE_SHEET_WIDTH_TILES);
	SASSERT(coord.y < TILE_SHEET_HEIGHT_TILES);

	uint16_t tileId = coord.x;
	tileId |= ((uint16_t)coord.y << 8);

	TileMgr.Tiles[tileId].IsUsed = true;

	return tileId;
}

TileData TileMgrCreate(uint16_t tileId)
{
	SASSERT(TileMgr.Tiles[tileId].IsUsed);

	uint8_t x = (uint8_t)tileId;
	uint8_t y = (uint8_t)(tileId >> 8);
	SASSERT(x < TILE_SHEET_WIDTH_TILES);
	SASSERT(y < TILE_SHEET_HEIGHT_TILES);

	TileData tile;
	tile.TexX = x;
	tile.TexY = y;
	tile.HasCeiling = false;
	tile.LOS = TileLOS::NoVision;
	return tile;
}

Tile* TileData::GetTile() const
{
	uint16_t id = TileMgrToTileId(AsCoord());
	return &TileMgr.Tiles[id];
}
