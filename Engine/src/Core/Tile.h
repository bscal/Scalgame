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

struct TileColor
{
	uint16_t r;
	uint16_t g;
	uint16_t b;
	uint16_t a;
	uint8_t Count;

	void AddColor(Color c);
	Vector4 FinalColor();

};

struct TileData
{
	std::string SpriteName;
	uint32_t TileId;
	int MovementCost;
	short Cover;
	TileType Type;
};

struct TileTexData
{
	Rectangle TexCoord;
};

#define MAX_TILES 32
struct TileMgr
{
	SpriteAtlas* SpriteAtlas;
	TileData Tiles[MAX_TILES];
	TileTexData TileTextureData[MAX_TILES];
	uint32_t NextTileId;
};

struct TileRenderInfo
{
	Vector4 Color;
	uint16_t TileId;
	TileLOS LOS;
};

struct Tile
{
	Vector4 Color;
	uint16_t TileId;
	TileLOS LOS;

	const TileData& GetTileData(TileMgr* tileMgr) const;
	const TileTexData& GetTileTexData(TileMgr* tileMgr) const;
};

void TileMgrInitialize(TileMgr* tileMgr, SpriteAtlas* spriteAtlas);

uint32_t RegisterTile(TileMgr* tileMgr,
	std::string_view spriteName,
	TileType type);

[[nodiscard]] Tile CreateTile(TileMgr* tileMgr, const TileData& tileData);
[[nodiscard]] Tile CreateTileId(TileMgr* tileMgr, uint32_t tileDataId);
