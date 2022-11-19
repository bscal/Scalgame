#pragma once

#include "Core.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

struct TileSet;
struct GameApplication;

typedef Vector2iu ChunkCoord;
typedef Vector2iu TileWorldCoord;

struct TileTextureCoord
{
	uint16_t x;
	uint16_t y;
};

struct TextureCoord
{
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
};

struct TileMapTile
{
	TextureCoord TextureCoord;
	uint32_t TileId;
	uint8_t FowLevel;
};

struct TileMapChunk
{
	SList<TileMapTile> Tiles;
	ChunkCoord ChunkCoord;
	bool IsChunkGenerated;
};

struct ThreadedTileMap
{
	SList<TileMapChunk> ChunksList;
	STable<ChunkCoord, TileMapChunk*> ChunksMap;
	SLinkedList<ChunkCoord> ChunksToLoad;
	SLinkedList<ChunkCoord> ChunksToUnload;
	TileSet* TileSet;
	Vector2iu TileMapDimensionsInChunks;
	Vector2iu ChunkDimensionsInTiles;
	uint64_t ChunkSize;
	float LoadDistance;
};

void ThreadedTileMapInitialize(ThreadedTileMap* tilemap, TileSet* tileSet,
	Vector2iu tileMapDimensionsInChunks, Vector2iu chunkDimensionsInTiles);
void ThreadedTileMapFree(ThreadedTileMap* tilemap);

void ThreadedTileMapCreate(ThreadedTileMap* tilemap, int loadWidth,
	int loadHeight);

void ThreadedTileMapUpdate(ThreadedTileMap* tilemap, GameApplication* game);
void UpdateChunk(ThreadedTileMap* tilmap,
	TileMapChunk* chunk, GameApplication* game);

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ThreadedTileMap* tilemap, ChunkCoord coord);
bool IsChunkLoaded(ThreadedTileMap* tilemap, ChunkCoord coord);

void GenerateChunk(ThreadedTileMap* tilemap, TileMapChunk* chunk);

TileMapChunk* GetChunk(ThreadedTileMap* tilemap, ChunkCoord coord);

ChunkCoord TileToChunk(ThreadedTileMap* tilemap, 
	uint64_t tileX, uint64_t tileY);

uint32_t TileToChunkTileIndex(ThreadedTileMap* tilemap,
	uint64_t tileX, uint64_t tileY);

void SetTile(ThreadedTileMap* tilemap, TileMapTile* tile,
	uint64_t tileX, uint64_t tileY);

TileMapTile* GetTile(ThreadedTileMap* tilemap,
	uint64_t tileX, uint64_t tileY);

