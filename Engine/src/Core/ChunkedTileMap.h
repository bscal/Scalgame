#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Tile.h"
#include "Scheduler.h"

#include "Structures/SHashMap.h"
#include "Structures/SLinkedList.h"
#include "Structures/StaticArray.h"

struct GameApp;
struct Game;

global_var const Vector2i TILEMAP_ORIGIN = { 0, 0 };

#define CHUNK_BAKE_FLAG_REBAKE_NEIGHBORS 1

constexpr global_var int CHUNK_SIDE_FLAG_NORTH	= (1 << 0);
constexpr global_var int CHUNK_SIDE_FLAG_EAST	= (1 << 1);
constexpr global_var int CHUNK_SIDE_FLAG_SOUTH	= (1 << 2);
constexpr global_var int CHUNK_SIDE_FLAG_WEST	= (1 << 3);
constexpr global_var int CHUNK_SIDE_FLAG_ALL	= (CHUNK_SIDE_FLAG_NORTH | CHUNK_SIDE_FLAG_EAST | CHUNK_SIDE_FLAG_SOUTH | CHUNK_SIDE_FLAG_WEST);

constexpr int GetNearSides(Vector2i v0, Vector2i v1)
{
	int xd = v0.x - v1.x;
	int yd = v0.y - v1.y;

	int res = 0;

	if (xd > 0)
		res |= CHUNK_SIDE_FLAG_EAST;
	if (xd < 0)
		res |= CHUNK_SIDE_FLAG_WEST;
	if (yd > 0)
		res |= CHUNK_SIDE_FLAG_SOUTH;
	if (yd < 0)
		res |= CHUNK_SIDE_FLAG_NORTH;

	return res;
}

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
	Vector2 ChunkCenter;
	Vector2i ChunkStartXY;
	ChunkCoord ChunkCoord;
	ChunkState State;
	StaticArray<TileData, CHUNK_SIZE> Tiles;
	StaticArray<uint32_t, CHUNK_SIZE> TileUpdateIds;
	StaticArray<Color, CHUNK_SIZE> TileColors;
};

struct ChunkedTileMap
{
	SHashMap<Vector2i, TileMapChunk> Chunks;
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

void BakeChunkLighting(ChunkedTileMap* tilemap, TileMapChunk* chunk, int chunkBakeFlags, int chunkSideFlags);

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