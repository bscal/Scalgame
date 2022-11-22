#include "TileData.h"

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

Tile CreateTile(TileMgr* tileMgr, uint32_t tileId)
{
	assert(tileMgr);
	assert(tileId < MAX_TILES);

	if (tileId >= tileMgr->NextTileId)
	{
		S_LOG_ERR("Attempting to create tile with unknown id: %d", tileId);
		return {};
	}
	
	TileData tileData = tileMgr->Tiles[tileId];

	Tile tile = {};
	tile.TileDataId = tileId;
	tile.TextureRect = tileData.TextureRect;
	return tile;
}