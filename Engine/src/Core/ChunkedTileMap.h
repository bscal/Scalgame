#pragma once

#include "Core.h"
#include "Globals.h"
#include "Vector2i.h"
#include "Tile.h"

#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

struct GameApp;
struct Game;

namespace CTileMap
{

struct TileMapChunk
{
	Rectangle Bounds;
	ChunkCoord ChunkCoord;
	SList<uint64_t> Entities;
	bool IsChunkGenerated;
	Tile Tiles[CHUNK_SIZE];
};

struct ChunkedTileMap
{
	// TODO temp removed map part, lookup speed
	// in not that important yet
	SList<TileMapChunk> ChunksList;
	SLinkedList<ChunkCoord> ChunksToUnload;

	Rectangle WorldBounds;
	Rectangle ChunkBounds;

	Vector2i TileSize;
	Vector2i Origin;
	Vector2i WorldDimChunks;
	Vector2i ChunkSize;

	Vector2i WorldDimTiles;
	Vector2i ViewDistance;
	size_t ChunkTileCount;
};

void Initialize(ChunkedTileMap* tilemap);
void Free(ChunkedTileMap* tilemap);

void 
FindChunksInView(ChunkedTileMap* tilemap, Game* game, Vector2i centerChunkCoord);

void Update(ChunkedTileMap* tilemap, Game* game);

TileMapChunk* 
LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

void GenerateChunk(ChunkedTileMap* tilemap,TileMapChunk* chunk);
void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
void UpdateChunk(ChunkedTileMap* tilmap, TileMapChunk* chunk, Game* game);
void LateUpdateChunk(ChunkedTileMap* tilemap, Game* game);

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

TileMapChunk*
GetChunkByTile(ChunkedTileMap* tilemap, ChunkCoord coord);

ChunkCoord
TileToChunkCoord(ChunkedTileMap* tilemap, TileCoord tilePos);

void 
SetTile(ChunkedTileMap* tilemap, const Tile* tile, TileCoord tilePos);

Tile* 
GetTile(ChunkedTileMap* tilemap, TileCoord tilePos);

const Tile& 
GetTileRef(ChunkedTileMap* tilemap, TileCoord tilePos);

ChunkCoord
TileToChunkCoord(ChunkedTileMap* tilemap, TileCoord tilePos);

uint64_t
TileToIndex(ChunkedTileMap* tilemap, TileCoord tilePos);

TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos);

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsTileInBounds(ChunkedTileMap* tilemap, TileCoord tilePos);
bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos);
void SetColor(ChunkedTileMap* tilemap, TileCoord coord, Vector4 color);
void SetVisible(ChunkedTileMap* tilemap, TileCoord coord);
bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord);

}