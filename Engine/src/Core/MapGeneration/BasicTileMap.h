#pragma once

#include "Core/Core.h"
#include "Core/Tile.h"
#include "Core/Vector2i.h"

#include "Core/Structures/SList.h"

struct TileMap
{
	SList<TileData> Tiles;
	Rectangle SourceRect;
	Vector2i Dimensions;
	bool IsLoaded;
	bool IsGenerated;
	
	void Free();

	void Load(uint32_t width, uint32_t height);
	void Unload();

	void Generate();

	TileData* GetTile(TileCoord tileCoord);
	void SetTile(TileCoord tileCoord, const TileData* tile);

	bool IsInBounds(TileCoord tileCoord);
};
