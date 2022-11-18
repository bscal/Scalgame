#include "ThreadedTileMap.h"

#include "SRandom.h"

#include <assert.h>

// TODO
global_var SRandom Random;

internal uint64_t ChunkMapHash(const ChunkCoord* key)
{
	return static_cast<uint64_t>(key->ToUInt64());
}

internal bool ChunkMapEquals(const ChunkCoord* lhs, const ChunkCoord* rhs)
{
	return lhs->AreEquals(*rhs);
}

void ThreadedTileMapInitialize(ThreadedTileMap* tilemap, TileSet* tileSet,
	Vector2iu tileMapDimensionsInChunks, Vector2iu chunkDimensionsInTiles)
{
	if (!tilemap)
	{
		TraceLog(LOG_ERROR, "ThreadedTileMapInitialize: tilemap cannot be nullptr");
		return;
	}
	if (!tileSet)
	{
		TraceLog(LOG_ERROR, "ThreadedTileMapInitialize: tileSet cannot be nullptr");
		return;
	}

	SRandomInitialize(&Random, 0);
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
		TraceLog(LOG_ERROR, "ThreadedTileMapFree: tilemap cannot be nullptr");
		return;
	}

	tilemap->ChunksList.Free();
	tilemap->ChunksMap.Free();
	SLinkedFree<Vector2i>(&tilemap->ChunksToUnload);
	SLinkedFree<Vector2i>(&tilemap->ChunksToLoad);
}

void ThreadedTileMapCreate(ThreadedTileMap* tilemap, int loadWidth,
	int loadHeight)
{
	for (int y = 0; y < loadHeight; ++y)
	{
		for (int x = 0; x < loadWidth; ++x)
		{
			ChunkCoord chunkCoord;
			chunkCoord.x = x;
			chunkCoord.y = y;
			TileMapChunk* chunk = LoadChunk(tilemap, chunkCoord);
			assert(chunk);
			GenerateChunk(tilemap, chunk);
		}
	}
}

void ThreadedTileMapUpdate(ThreadedTileMap* tilemap, GameApplication* game)
{
	assert(tilemap);
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		UpdateChunk(tilemap, tilemap->ChunksList.PeekAtPtr(i), game);
	}
}

void UpdateChunk(ThreadedTileMap* tilemap,
	TileMapChunk* chunk, GameApplication* game)
{
	assert(chunk);
	assert(chunk->IsChunkGenerated);
	for (uint32_t y = 0; y < tilemap->ChunkDimensionsInTiles.y; ++y)
	{
		for (uint32_t x = 0; x < tilemap->ChunkDimensionsInTiles.x; ++x)
		{
			size_t index = x + y * tilemap->ChunkDimensionsInTiles.x;
			TileMapTile tile = chunk->Tiles[index];

			Rectangle textureRect;
			textureRect.x = (float)tile.TextureCoord.X;
			textureRect.y = (float)tile.TextureCoord.Y;
			textureRect.width = 16.0f;
			textureRect.height = 16.0f;

			int worldX = chunk->ChunkCoord.x + x;
			int worldY = chunk->ChunkCoord.y + y;
			Vector2 worldPosition;
			worldPosition.x = (float)worldX * 16.0f;
			worldPosition.y = (float)worldY * 16.0f;

			DrawTextureRec(
				tilemap->TileSet->TileTexture,
				textureRect,
				worldPosition,
				WHITE);
		}
	}
}

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk chunk = {};
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
	for (size_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		if (tilemap->ChunksList[i].ChunkCoord == coord)
		{
			tilemap->ChunksList.RemoveAtFast(i);
		}
	}
}

void GenerateChunk(ThreadedTileMap* tilemap, TileMapChunk* chunk)
{
	assert(chunk);
	if (chunk->IsChunkGenerated)
	{
		S_LOG_WARN("Trying to generate an already generated chunk!");
		return;
	}
	chunk->IsChunkGenerated = true;

	uint32_t index = 0;
	for (uint32_t y = 0; y < tilemap->ChunkDimensionsInTiles.y; ++y)
	{
		for (uint32_t x = 0; x < tilemap->ChunkDimensionsInTiles.x; ++x)
		{
			uint32_t tileId = (uint32_t)SRandNextRange(&Random, 1, 2);
			TileMapTile tile = {};
			tile.TileId = tileId;
			uint16_t texX = tileId % 16;
			uint16_t texY = tileId / 16;
			tile.TextureCoord = { texX, texY };
			tile.FowLevel = 0;
			chunk->Tiles[index++] = tile;
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

ChunkCoord TileToChunk(ThreadedTileMap* tilemap,
	int64_t tileX, int64_t tileY)
{
	int chunkX = tileX / (int64_t)tilemap->TileMapDimensionsInChunks.x;
	int chunkY = tileY / (int64_t)tilemap->TileMapDimensionsInChunks.y;
	return { chunkX, chunkY };
}

uint32_t TileToChunkTileIndex(ThreadedTileMap* tilemap,
	int64_t tileX, int64_t tileY)
{
	uint64_t tileChunkX = tileX % (int64_t)tilemap->ChunkDimensionsInTiles.x;
	uint64_t tileChunkY = tileY % (int64_t)tilemap->ChunkDimensionsInTiles.y;
	return tileChunkX + tileChunkY * tilemap->ChunkDimensionsInTiles.x;
}

void SetTile(ThreadedTileMap* tilemap, TileMapTile* tile,
	int64_t tileX, int64_t tileY)
{
	ChunkCoord chunkCoord = TileToChunk(tilemap, tileX, tileY);
	uint32_t tileChunkCoord = TileToChunkTileIndex(tilemap, tileX, tileY);
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&chunkCoord);
	if (!chunk)
	{
		#if SCAL_DEBUG
			assert(false);
		#endif
		return;
	}
	assert(((*chunk)->Tiles.IsInitialized()));
	(*chunk)->Tiles[tileChunkCoord] = *tile;
}

TileMapTile* GetTile(ThreadedTileMap* tilemap,
	int64_t tileX, int64_t tileY)
{
	ChunkCoord chunkCoord = TileToChunk(tilemap, tileX, tileY);
	uint32_t tileChunkCoord = TileToChunkTileIndex(tilemap, tileX, tileY);
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&chunkCoord);
	if (!chunk)
	{
		#if SCAL_DEBUG
			assert(false);
		#endif
		return nullptr;
	}
	assert(((*chunk)->Tiles.IsInitialized()));
	return (*chunk)->Tiles.PeekAtPtr(tileChunkCoord);
}