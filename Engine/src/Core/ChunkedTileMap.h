#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

struct TileSet;
struct GameApplication;

typedef Vector2i ChunkCoord;
typedef Vector2i TileWorldCoord;

namespace ChunkedTileMap
{

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

struct ChunkedTileMap
{
	SList<TileMapChunk> ChunksList;
	STable<ChunkCoord, TileMapChunk*> ChunksMap;
	SLinkedList<ChunkCoord> ChunksToLoad;
	SLinkedList<ChunkCoord> ChunksToUnload;
	TileSet* TileSet;
	Vector2i TileMapDimensionsInChunks;
	Vector2i ChunkDimensionsInTiles;
	uint64_t WorldBoundsX;
	uint64_t WorldBoundsY;
	uint64_t ChunkSize;
	Vector2 ViewDistanceInChunk;
};

void Initialize(ChunkedTileMap* tilemap, TileSet* tileSet,
	Vector2i tileMapDimensionsInChunks, Vector2i chunkDimensionsInTiles);
void Free(ChunkedTileMap* tilemap);

void Create(ChunkedTileMap* tilemap, int loadWidth,
	int loadHeight);

void FindChunksInView(ChunkedTileMap* tilemap, GameApplication* gameApp);

void Update(ChunkedTileMap* tilemap, GameApplication* gameApp);
void UpdateChunk(ChunkedTileMap* tilmap,
	TileMapChunk* chunk, GameApplication* gameApp);

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);

void GenerateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk);

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

ChunkCoord GetWorldTileToChunkCoord(ChunkedTileMap* tilemap,
	uint64_t tileX, uint64_t tileY);

uint32_t GetWorldTileToChunkIndex(ChunkedTileMap* tilemap,
	uint64_t tileX, uint64_t tileY);

void SetTile(ChunkedTileMap* tilemap, TileMapTile* tile,
	uint64_t tileX, uint64_t tileY);

TileMapTile* GetTile(ChunkedTileMap* tilemap,
	uint64_t tileX, uint64_t tileY);

bool IsChunkInBounds(ChunkedTileMap* tilemap, uint32_t chunkX, uint32_t chunkY);

bool IsTileInBounds(ChunkedTileMap* tilemap, uint64_t worldTileX, uint64_t worldTileY);

}