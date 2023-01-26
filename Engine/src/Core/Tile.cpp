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

uint32_t RegisterTile(TileMgr* tileMgr,
	const char* spriteName,
	TileType type)
{
	assert(tileMgr);
	assert(tileMgr->NextTileId < MAX_TILES);
	assert(spriteName[0] != 0);

	uint32_t id = tileMgr->NextTileId++;

	tileMgr->Tiles[id].TileId = id;
	tileMgr->Tiles[id].SpriteName = spriteName;
	tileMgr->Tiles[id].Type = type;

	tileMgr->TileTextureData[id].TexCoord = 
		tileMgr->SpriteAtlas->GetRectByName(SStringView(spriteName, strlen(spriteName)));

	return id;
}


[[nodiscard]] Tile CreateTile(TileMgr* tileMgr, const TileData& tileData)
{
	SASSERT(tileMgr);
	Tile tile;
	tile.TileId = tileData.TileId;
	tile.LOS = TileLOS::NoVision;
	return tile;
}

[[nodiscard]] Tile CreateTileId(TileMgr* tileMgr, uint32_t tileId)
{
	SASSERT(tileMgr);
	SASSERT(tileId < MAX_TILES);
	return CreateTile(tileMgr, tileMgr->Tiles[tileId]);
}

const TileData* const Tile::GetTileData(TileMgr* tileMgr) const
{
	return &tileMgr->Tiles[TileId];
}

const TileTexData* const Tile::GetTileTexData(TileMgr* tileMgr) const
{
	return &tileMgr->TileTextureData[TileId];
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
