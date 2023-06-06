#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Tile.h"

#include "Structures/SHoodTable.h"
#include "Structures/SLinkedList.h"
#include "Structures/StaticArray.h"

struct GameApp;
struct Game;

global_var const Vector2i TILEMAP_ORIGIN = { 0, 0 };

enum class ChunkState : uint8_t
{
	Unloaded = 0,
	Loaded,
	Sleeping,

	MaxStates
};

struct TileColor
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct TileMapChunk
{
	Rectangle Bounds;
	ChunkCoord ChunkCoord;
	ChunkState State;
	StaticArray<TileData, CHUNK_SIZE> Tiles;
	//TileColor Colors[CHUNK_SIZE];
};

struct ChunkedTileMap
{
	SHoodTable<Vector2i, TileMapChunk, Vector2iHasher, Vector2iEquals> Chunks;
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

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
TileMapChunk* GetChunkByTile(ChunkedTileMap* tilemap, TileCoord tileCoord);
ChunkCoord TileToChunkCoord(TileCoord tilePos);

size_t TileToIndex(TileCoord tilePos);
TileCoord WorldToTile(Vector2 pos);

void SetTile(ChunkedTileMap* tilemap, const TileData* tile, TileCoord tilePos);
TileData* GetTile(ChunkedTileMap* tilemap, TileCoord tilePos);

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsTileInBounds(ChunkedTileMap* tilemap, TileCoord tilePos);
bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos);

void SetVisible(ChunkedTileMap* tilemap, TileCoord coord);
bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord);

}