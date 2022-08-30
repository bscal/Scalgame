#pragma once

#include <Engine.h>

struct Game;

struct TexturedTile
{
	Rectangle TextureSrcRectangle;
};

struct Tile
{
	uint32_t TileId;
};

struct TileMap
{
	uint32_t Width;
	uint32_t Height;
	uint16_t TileSize;
	Texture2D* Texture;
	Tile* MapTiles;
	TexturedTile* TexturedTiles;
};

bool InitializeTileMap(
	Texture2D* Texture,
	uint32_t width,
	uint32_t height,
	uint16_t tileSize,
	TileMap* outData);

void LoadTileMap(TileMap* tileMap);

void UnloadTileMap(TileMap* tileMap);
void RenderTileMap(Game* game, TileMap* tileMap);

Tile* GetTile(TileMap* tileMap, int x, int y);
void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile);
