#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Tile.h"
#include "Scheduler.h"

#include "Structures/SHashMap.h"
#include "Structures/SHashMapDense.h"
#include "Structures/SLinkedList.h"
#include "Structures/StaticArray.h"

struct GameApp;
struct Game;

global_var const Vector2i TILEMAP_ORIGIN = { 0, 0 };

#define CHUNK_REBAKE_SELF (1 << 0)
#define CHUNK_REBAKE_NEIGHBORS (1 << 1)
#define CHUNK_REBAKE_ALL (CHUNK_REBAKE_SELF | CHUNK_REBAKE_NEIGHBORS)

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
	DistributedTileUpdater TileUpdater;
	Vector2i StartTile;
	ChunkCoord ChunkCoord;
	ChunkState State;
	uint8_t RebakeFlags;
	bool IsBaked;
	StaticArray<TileData, CHUNK_SIZE> Tiles;
	StaticArray<Color, CHUNK_SIZE> TileColors;
};

struct ChunkedTileMap
{
	SHashMap<Vector2i, TileMapChunk*> Chunks;
	SLinkedList<ChunkCoord> ChunksToUnload;

	DistributedTileUpdater TileUpdater;

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

void UpdateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk);

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

void BakeChunkLighting(ChunkedTileMap* tilemap, TileMapChunk* chunk, int chunkBakeFlags);

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
TileMapChunk* GetChunkByTile(ChunkedTileMap* tilemap, TileCoord tileCoord);
ChunkCoord TileToChunkCoord(TileCoord tilePos);

size_t GetTileLocalIndex(TileCoord tilePos);
TileCoord WorldToTile(Vector2 pos);

void SetTile(ChunkedTileMap* tilemap, const TileData* tile, TileCoord tilePos);
TileData* GetTile(ChunkedTileMap* tilemap, TileCoord tilePos);

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsTileInBounds(ChunkedTileMap* tilemap, TileCoord tilePos);
bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos);

void SetVisible(ChunkedTileMap* tilemap, TileCoord coord);
bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord);

}