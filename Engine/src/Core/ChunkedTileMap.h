#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Tile.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/STable.h"

struct Game;

typedef Vector2i ChunkCoord;
typedef Vector2i TileCoord;

namespace CTileMap
{

struct TileMapChunk
{
	SList<Tile> Tiles;
	Rectangle Bounds;
	ChunkCoord ChunkCoord;
	bool IsChunkGenerated;
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
	Vector2i OriginTiles;
	Vector2i WorldDimChunks;
	Vector2i ChunkSize;

	Vector2i WorldDimTiles;
	Vector2i ViewDistance;
	size_t ChunkTileCount;
};

void Initialize(ChunkedTileMap* tilemap, Game* game,
	Vector2i tileSize, Vector2i origin,
	Vector2i worldDimChunks, Vector2i chunkSize);
void Free(ChunkedTileMap* tilemap);

void FindChunksInView(ChunkedTileMap* tilemap, Game* game);
void Update(ChunkedTileMap* tilemap, Game* game);

internal void Draw(ChunkedTileMap* tilemap, Game* game);

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord);
internal void CreateChunk(ChunkedTileMap* tilemap,
	int loadWidth, int loadHeight);
void GenerateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk);

void UnloadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord);
void UpdateChunk(ChunkedTileMap* tilmap,
	TileMapChunk* chunk, Game* game);
void LateUpdateChunk(ChunkedTileMap* tilemap,
	Game* game);

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord);
bool IsTileInBounds(ChunkedTileMap* tilemap,
	TileCoord tilePos);
bool IsChunkInBounds(ChunkedTileMap* tilemap,
	ChunkCoord chunkPos);

TileMapChunk* GetChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord);
void SetTile(ChunkedTileMap* tilemap, const Tile* tile,
	TileCoord tilePos);
Tile* GetTile(ChunkedTileMap* tilemap,
	TileCoord tilePos);
const Tile& GetTileRef(ChunkedTileMap* tilemap,
	TileCoord tilePos);
Tile* GetTileChunk(TileMapChunk* chunk,
	TileCoord tilePos);

ChunkCoord TileToChunkCoord(ChunkedTileMap* tilemap,
	TileCoord tilePos);
uint64_t TileToIndex(ChunkedTileMap* tilemap,
	TileCoord tilePos);
uint64_t TileToIndexLocal(Vector2i origin, uint64_t width,
	TileCoord tilePos);

TileCoord IndexToTile(ChunkedTileMap* tilemap,
	uint64_t index);

TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos);

}