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
	uint64_t TileId;
};

struct TileMapChunk
{
	SList<TileMapTile> Tiles;
	ChunkCoord ChunkCoord;
};

struct ThreadedTileMap
{
	SList<TileMapChunk> ChunksList;
	STable<ChunkCoord, TileMapChunk*> ChunksMap;
	SLinkedList<ChunkCoord> ChunksToLoad;
	SLinkedList<ChunkCoord> ChunksToUnload;
	Vector2i TileMapDimensionsInChunks;
	Vector2i ChunkDimensionsInTiles;
	uint32_t ChunkSize;
	float LoadDistance;
};

void ThreadedTileMapInitialize(ThreadedTileMap* tilemap, Vector2i tileMapDimensionsInChunks,
	Vector2i chunkDimensionsInTiles);
void ThreadedTileMapFree(ThreadedTileMap* tilemap);

void ThreadedTileMapUpdate(ThreadedTileMap* tilemap, GameApplication* game);

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
bool IsChunkLoaded(ThreadedTileMap* tilemap, ChunkCoord coord);

TileMapChunk* GetChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
