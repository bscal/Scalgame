#include "Tile.h"

#include "assert.h"

void TileMgrInitialize(TileMgr* tileMgr, SpriteAtlas* spriteAtlas)
{
	assert(tileMgr);
	assert(spriteAtlas);
	tileMgr->SpriteAtlas = spriteAtlas;

	RegisterTile(tileMgr, "Tile1", TileType::Floor);
	RegisterTile(tileMgr, "Tile2", TileType::Floor);
	RegisterTile(tileMgr, "Tile3", TileType::Floor);
	RegisterTile(tileMgr, "Tile4", TileType::Floor);
	RegisterTile(tileMgr, "Tile5", TileType::Solid);
	RegisterTile(tileMgr, "Tile6", TileType::Solid);

	SLOG_INFO("TileManager Initialized! %d tiles registered.",
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
		SLOG_ERR("Could not find tile by name!");
		return {};
	}
	const auto& textCoord = tileMgr->SpriteAtlas->GetRect(find->second);

	Tile tile = {};
	tile.TextureRect = textCoord;
	tile.TileDataId = tileData.TileId;
	tile.TileColor = {};
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
		SLOG_ERR("Could not find tile by name!");
		return {};
	}
	const auto& textCoord = tileMgr->SpriteAtlas->GetRect(find->second);

	Tile tile = {};
	tile.TextureRect = textCoord;
	tile.TileDataId = tileDataId;
	tile.TileColor = {};
	return tile;
}

const TileData& Tile::GetTileData(TileMgr* tileMgr) const
{
	return tileMgr->Tiles[TileDataId];
}

void TileColor::AddColor(Color c)
{
	r += (uint16_t)c.r;
	b += (uint16_t)c.g;
	b += (uint16_t)c.b;
	a += (uint16_t)c.a;
	++Count;
}

Vector4 TileColor::FinalColor()
{
	if (Count == 0)
		return {};

	uint16_t finalR = r / (uint16_t)Count;
	uint16_t finalG = g / (uint16_t)Count;
	uint16_t finalB = b / (uint16_t)Count;
	uint16_t finalA = a / (uint16_t)Count;
	
	r = 0;
	g = 0;
	b = 0;
	a = 0;
	Count = 0;

	return { (float)finalR / 255.0f, (float)finalG / 255.0f, (float)finalB / 255.0f, (float)finalA / 255.0f };
}
