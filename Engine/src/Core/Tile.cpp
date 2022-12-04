#include "Tile.h"

#include "assert.h"

bool TileMgrRegisterTile(TileMgr* tileMgr, TileData* tileData)
{
	assert(tileMgr);
	assert(tileData);

	if (tileMgr->NextTileId == MAX_TILES)
	{
		S_LOG_ERR("Registering tileData but max tiles already reached!");
		return false;
	}

	tileData->TileId = tileMgr->NextTileId++;
	tileMgr->Tiles[tileData->TileId] = *tileData;
	return true;
}

Tile CreateTile(TileMgr* tileMgr, TileId tileId)
{
	assert(tileMgr);
	assert(tileId < MAX_TILES);

	if (tileId >= tileMgr->NextTileId)
	{
		S_LOG_ERR("Attempting to create tile with unknown id: %d", tileId);
		return {};
	}
	
	const auto& tileData = GetTileData(tileMgr, tileId);

	Tile tile = {};
	tile.TileDataId = tileId;
	tile.TextureRect = tileData.TextureRect;
	return tile;
}

void SetTile(TileMgr* tileMgr, Tile* tile, TileId newTileId)
{
	assert(tileMgr);
	assert(newTileId < MAX_TILES);

	if (newTileId == tile->TileDataId) return;

	const auto& tileData = GetTileData(tileMgr, newTileId);

	tile->TileDataId = newTileId;
	tile->TextureRect = tileData.TextureRect;
}

const TileData& GetTileData(TileMgr* tileMgr, TileId tileId)
{
	return tileMgr->Tiles[tileId];
}