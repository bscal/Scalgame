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
	Rectangle TextureCoord;
	uint32_t TileId;
	uint8_t FowLevel;
};

struct TileMapChunk
{
	SList<TileMapTile> Tiles;
	Rectangle Bounds;
	ChunkCoord ChunkCoord;
	bool IsChunkGenerated;
};

struct ChunkedTileMap
{
	SList<TileMapChunk> ChunksList;
	STable<ChunkCoord, TileMapChunk*> ChunksMap;
	SLinkedList<ChunkCoord> ChunksToUnload;
	TileSet* TileSet;
	Vector2i BoundsStart;
	Vector2i BoundsEnd;
	Vector2i StartChunkCoords;
	Vector2i EndChunkCoords;
	Vector2i ChunkDimensions;
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
void LateUpdateChunk(ChunkedTileMap* tilemap, GameApplication* gameApp);

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);

void GenerateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk);

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

ChunkCoord GetWorldTileToChunkCoord(ChunkedTileMap* tilemap,
	int tileX, int tileY);

uint64_t GetWorldTileToChunkIndex(ChunkedTileMap* tilemap,
	int tileX, int tileY);

void SetTile(ChunkedTileMap* tilemap, TileMapTile* tile,
	int tileX, int tileY);

TileMapTile* GetTile(ChunkedTileMap* tilemap,
	int tileX, int tileY);

}