#include "Tile.h"

#include "assert.h"

void TileMgrInitialize(TileMgr* tileMgr, SpriteAtlas* spriteAtlas)
{
	assert(tileMgr);
	assert(spriteAtlas);
	tileMgr->SpriteAtlas = spriteAtlas;

	RegisterTile(tileMgr, "Tile1", TileType::Floor);
	RegisterTile(tileMgr, "Tile2", TileType::Floor);
	RegisterTile(tileMgr, "Tile3", TileType::Solid);
	RegisterTile(tileMgr, "Tile4", TileType::Floor);
	RegisterTile(tileMgr, "Tile5", TileType::Solid);
	RegisterTile(tileMgr, "Tile6", TileType::Solid);

	S_LOG_INFO("TileManager Initialized! %d tiles registered.",
		tileMgr->NextTileId);
}

TileData& RegisterTile(TileMgr* tileMgr,
	std::string_view spriteName,
	TileType type)
{
	assert(tileMgr);
	assert(tileMgr->NextTileId < MAX_TILES);
	assert(spriteName.length() > 0);

	uint32_t id = tileMgr->NextTileId++;

	tileMgr->Tiles[id].TileId = id;
	tileMgr->Tiles[id].SpriteName = spriteName;
	tileMgr->Tiles[id].Type = type;
	return tileMgr->Tiles[id];
}


[[nodiscard]] Tile CreateTile(TileMgr* tileMgr, const TileData& tileData)
{
	assert(tileMgr);

	auto find = tileMgr->SpriteAtlas->SpritesByName.find(
		tileData.SpriteName);
	if (find == tileMgr->SpriteAtlas->SpritesByName.end())
	{
		S_LOG_ERR("Could not find tile by name!");
		return {};
	}
	const auto& textCoord = tileMgr->SpriteAtlas->GetRect(find->second);

	Tile tile = {};
	tile.TextureRect = textCoord;
	tile.TileDataId = tileData.TileId;
	tile.TileColor = WHITE;
	return tile;
}

[[nodiscard]] Tile CreateTileId(TileMgr* tileMgr, uint32_t tileDataId)
{
	assert(tileMgr);
	assert(tileDataId < tileMgr->NextTileId);

	const std::string& name = tileMgr->Tiles[tileDataId].SpriteName;
	
	assert(!name.empty());

	auto find = tileMgr->SpriteAtlas->SpritesByName.find(
		name);
	if (find == tileMgr->SpriteAtlas->SpritesByName.end())
	{
		S_LOG_ERR("Could not find tile by name!");
		return {};
	}
	const auto& textCoord = tileMgr->SpriteAtlas->GetRect(find->second);

	Tile tile = {};
	tile.TextureRect = textCoord;
	tile.TileDataId = tileDataId;
	tile.TileColor = WHITE;
	return tile;
}

const TileData& Tile::GetTileData(TileMgr* tileMgr) const
{
	return tileMgr->Tiles[TileDataId];
}