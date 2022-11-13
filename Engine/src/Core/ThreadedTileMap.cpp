#include "ThreadedTileMap.h"


internal uint64_t ChunkMapHash(const ChunkCoord* key)
{
	return key->ToInt64();
}

internal bool ChunkMapEquals(const ChunkCoord* lhs, const ChunkCoord* rhs)
{
	return *lhs == *rhs;
}

void ThreadedTileMapInitialize(ThreadedTileMap* tilemap, Vector2i tileMapDimensionsInChunks,
	Vector2i chunkDimensionsInTiles)
{
	if (!tilemap)
	{
		TraceLog(LOG_ERROR, "tilemap cannot be nullptr");
		return;
	}

	tilemap->TileMapDimensionsInChunks = tileMapDimensionsInChunks;
	tilemap->ChunkDimensionsInTiles = chunkDimensionsInTiles;
	tilemap->ChunkSize = chunkDimensionsInTiles.x * chunkDimensionsInTiles.y;

	tilemap->ChunksList.Initialize();
	tilemap->ChunksMap.InitializeEx(16, &ChunkMapHash, &ChunkMapEquals);
}

void ThreadedTileMapFree(ThreadedTileMap* tilemap)
{
	if (!tilemap)
	{
		TraceLog(LOG_ERROR, "tilemap cannot be nullptr");
		return;
	}

	tilemap->ChunksList.Free();
	tilemap->ChunksMap.Free();
	SLinkedFree<Vector2i>(&tilemap->ChunksToUnload);
	SLinkedFree<Vector2i>(&tilemap->ChunksToLoad);
}

void ThreadedTileMapUpdate(ThreadedTileMap* tilemap, GameApplication* game)
{
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
	}
}

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk chunk;
	chunk.Tiles.InitializeCap(tilemap->ChunkSize);
	chunk.ChunkCoord = coord;

	tilemap->ChunksList.Push(&chunk);
	TileMapChunk* chunkPtrInChunksList = tilemap->ChunksList.Last();
	tilemap->ChunksMap.Put(&coord, &chunkPtrInChunksList);
	return chunkPtrInChunksList;
}

void UnloadChunk(ThreadedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&coord);
	if (!chunk) return;

	tilemap->ChunksMap.Remove(&coord);

	// TODO maybe look into storing index in Chunk, and swapping
	// with last chunk here?
	for (int i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		if (tilemap->ChunksList[i].ChunkCoord == coord)
		{
			tilemap->ChunksList.RemoveAtFast(i);
		}
	}
}


bool IsChunkLoaded(ThreadedTileMap* tilemap, ChunkCoord coord)
{
	return tilemap->ChunksMap.Contains(&coord);
}

TileMapChunk* GetChunk(ThreadedTileMap* tilemap, ChunkCoord coord)
{
	return *tilemap->ChunksMap.Get(&coord);
}