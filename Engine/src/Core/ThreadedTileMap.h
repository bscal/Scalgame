#pragma once

#include "Core.h"
#include "Game.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

typedef Vector2i ChunkCoord;

struct TileCoord
{
	uint16_t X;
	uint16_t Y;
};

struct TileMapTile
{
	TileCoord TextureCoord;
	uint8_t FowLevel;
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
	TileSet* TileSet;
	Vector2i TileMapDimensionsInChunks;
	Vector2i ChunkDimensionsInTiles;
	uint32_t ChunkSize;
	float LoadDistance;
};

void ThreadedTileMapInitialize(ThreadedTileMap* tilemap, TileSet* tileSet,
	Vector2i tileMapDimensionsInChunks, Vector2i chunkDimensionsInTiles);
void ThreadedTileMapFree(ThreadedTileMap* tilemap);

void ThreadedTileMapUpdate(ThreadedTileMap* tilemap, GameApplication* game);
void UpdateChunk(ThreadedTileMap* tilmap,
	TileMapChunk* chunk, GameApplication* game);

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
bool IsChunkLoaded(ThreadedTileMap* tilemap, ChunkCoord coord);

TileMapChunk* GetChunk(ThreadedTileMap* tilemap, ChunkCoord coord);

