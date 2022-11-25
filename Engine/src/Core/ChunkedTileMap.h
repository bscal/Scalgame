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
	SLinkedList<ChunkCoord> ChunksToUnload;
	TileSet* TileSet;
	Vector2i StartPos;
	Vector2i EndPos;
	Vector2i WorldDimensionsInTiles;
	Vector2i ChunkDimensionsInTiles;
	Vector2 ViewDistance;
	size_t ChunkTileSize;
};

void Initialize(ChunkedTileMap* tilemap, TileSet* tileSet, Vector2i mapStartPos,
	Vector2i mapEndPos, Vector2i chunkDimensions);
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

}