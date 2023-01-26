#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Tile.h"
#include "EntityMgr.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

struct GameApp;
struct Game;

global_var constexpr int CHUNK_SIZE = 64;
global_var constexpr int TILE_SIZE = 16;
global_var constexpr float HALF_TILE_SIZE = ((float)TILE_SIZE) / 2.0f;
global_var constexpr Vector2i VIEW_DISTANCE = { 3, 2 };

namespace CTileMap
{

struct TileMapChunk
{
	Rectangle Bounds;
	ChunkCoord ChunkCoord;
	SList<EntityId> Entities;
	bool IsChunkGenerated;
	Tile Tiles[CHUNK_SIZE * CHUNK_SIZE];
	Vector4 TileColors[CHUNK_SIZE * CHUNK_SIZE];
	Vector2 LastLightPos[CHUNK_SIZE * CHUNK_SIZE];
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

void Initialize(ChunkedTileMap* tilemap, Game* game);
void Free(ChunkedTileMap* tilemap);

void FindChunksInView(ChunkedTileMap* tilemap, Game* game);
void Update(ChunkedTileMap* tilemap, Game* game);

internal void 
Draw(ChunkedTileMap* tilemap, Game* game);

TileMapChunk* 
LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

internal void 
CreateChunk(ChunkedTileMap* tilemap, int loadWidth, int loadHeight);

void GenerateChunk(ChunkedTileMap* tilemap,TileMapChunk* chunk);

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord);
void UpdateChunk(ChunkedTileMap* tilmap, TileMapChunk* chunk, Game* game);
void LateUpdateChunk(ChunkedTileMap* tilemap, Game* game);

TileMapChunk*
GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord);

TileMapChunk*
GetChunkByTile(ChunkedTileMap* tilemap, ChunkCoord coord);

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
void SetVisible(ChunkedTileMap* tilemap, TileCoord coord);
bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord);

}