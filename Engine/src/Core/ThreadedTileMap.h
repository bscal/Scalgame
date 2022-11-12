#pragma once

#include "Core.h"
#include "Game.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

typedef Vector2i ChunkCoord;
typedef Vector2i TileCoord;

struct TileMapTile
{

};

struct TileMapChunk
{
	SList<TileMapTile> Tiles;
};

struct ThreadedTileMap
{
	SList<TileMapChunk> ChunksList;
	STable<ChunkCoord, TileMapChunk*> ChunksMap;
	SLinkedList<ChunkCoord> ChunksToLoad;
	SLinkedList<ChunkCoord> ChunksToUnload;
	Vector2i TileMapDimensionsInChunks;
	Vector2i ChunkDimensionsInTiles;
};

void TileMapInitialize(ThreadedTileMap* tilemap, Vector2i tileMapDimensionsInChunks,
	Vector2i chunkDimensionsInTiles);
void TileMapFree(ThreadedTileMap* tilemap);

void UpdateChunks(ThreadedTileMap* tilemap, GameApplication* game);

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
bool IsChunkLoaded(ThreadedTileMap* tilemap, ChunkCoord coord);

TileMapChunk* GetChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
