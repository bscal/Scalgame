#pragma once

#include "Core.h"
#include "Globals.h"
#include "Vector2i.h"
#include "Tile.h"

#include "Structures/SList.h"
#include "Structures/SLinkedList.h"

struct GameApp;
struct Game;

global_var const Vector2i TILEMAP_ORIGIN = { 0, 0 };

#define NO_REBUILD 0x00
#define REBUILD_SELF 0x01
#define REBUILD_NEIGHBOURS 0x02

enum class ChunkState : uint8_t
{
	Unloaded = 0,
	Loaded,
	Sleeping,

	MaxStates
};

struct TileMapChunk
{
	Rectangle Bounds;
	ChunkCoord ChunkCoord;
	ChunkState State;
	uint8_t RebakeFlags;
	Tile Tiles[CHUNK_SIZE];
};


struct ChunkedTileMap
{
	// TODO temp removed map part, lookup speed
	// in not that important yet
	SList<TileMapChunk> ChunksList;
	SLinkedList<ChunkCoord> ChunksToUnload;

	Vector2i ViewDistance;
	Vector2i WorldDimChunks;	// Used in bounds check
	Vector2i WorldDimTiles;		// Used in bounds check
};

namespace CTileMap
{

void Initialize(ChunkedTileMap* tilemap);
void Free(ChunkedTileMap* tilemap);
void Load(ChunkedTileMap* tilemap);

void Update(ChunkedTileMap* tilemap, Game* game);
void LateUpdate(ChunkedTileMap* tilemap, Game* game);

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

void UpdateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk, Game* game);

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
TileMapChunk* GetChunkByTile(ChunkedTileMap* tilemap, TileCoord tileCoord);
ChunkCoord TileToChunkCoord(ChunkedTileMap* tilemap, TileCoord tilePos);

uint64_t TileToIndex(ChunkedTileMap* tilemap, TileCoord tilePos);
TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos);

void SetTile(ChunkedTileMap* tilemap, const Tile* tile, TileCoord tilePos);
Tile* GetTile(ChunkedTileMap* tilemap, TileCoord tilePos);

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsTileInBounds(ChunkedTileMap* tilemap, TileCoord tilePos);
bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos);

void SetVisible(ChunkedTileMap* tilemap, TileCoord coord);
bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord);

}