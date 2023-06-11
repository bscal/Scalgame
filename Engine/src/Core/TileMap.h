#pragma once

#include "Core/Core.h"

#include "Structures/SLinkedList.h"
#include "Structures/SHashMap.h"
#include "Structures/StaticArray.h"
#include "Structures/BitArray.h"

struct Game;

struct TileInfo
{
	uint8_t X;
	uint8_t Y;
};

enum ChunkFlags : uint8_t
{
	REBUILD = 1,
	REBUILD_NEIGHBORS = 1 << 1,
};

struct TileChunk
{
	static constexpr size_t CHUNK_DIMENSION = 32;
	static constexpr size_t CHUNK_SIZE = CHUNK_DIMENSION * CHUNK_DIMENSION;

	RenderTexture2D RenderTexture;
	Rectangle BoundingBox;
	bool IsLoaded;
	bool IsGenerated;
	bool ShouldRebuild;
	BitArray<CHUNK_SIZE / SIZEOF_I64_BITS> TilesMarkedForRebuild;
	StaticArray<TileInfo, CHUNK_SIZE> Tiles;
};

struct TileMap
{
	SHashMap<Vector2i, TileChunk> Chunks;
	SLinkedList<Vector2i> ChunksToUnload;
	Vector2i ViewDistanceInChunks;
	Vector2i WorldDimensionsInChunks;

	void Initialize();
	void Free();

	void UpdateChunks(Game* game);
	void CullAndDrawChunks(Game* game, RenderTexture2D target);
	TileChunk* LoadChunk(Vector2i coord);
	void UnloadChunk(Vector2i coord);
	void BuildChunk(TileChunk* chunk);

	TileInfo* GetTile(Vector2i tile);
	void SetTile(Vector2i tile, const TileInfo* data, ChunkFlags flags);

	Vector2i GetTileChunkCoord(Vector2i tile);
	uint32_t GetTileIndex(Vector2i tile);
};
