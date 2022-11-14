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
		UpdateChunk(tilemap, tilemap->ChunksList.PeekAtPtr(i), game);
	}
}

void UpdateChunk(ThreadedTileMap* tilemap,
	TileMapChunk* chunk, GameApplication* game)
{
	for (uint32_t y = 0; y < tilemap->ChunkDimensionsInTiles.y; ++y)
	{
		for (uint32_t x = 0; x < tilemap->ChunkDimensionsInTiles.x; ++x)
		{
			uint32_t worldX = chunk->ChunkCoord.x + x;
			uint32_t worldY = chunk->ChunkCoord.y + y;
			uint32_t index = x + y * tilemap->ChunkDimensionsInTiles.x;
			TileMapTile tile = chunk->Tiles[index];

			Rectangle textureRect;
			textureRect.x = tile.TextureCoord.X;
			textureRect.y = tile.TextureCoord.Y;
			textureRect.width = 16.0f;
			textureRect.height = 16.0f;

			Vector2 worldPosition;
			worldPosition.x = worldX * 16.0f;
			worldPosition.y = worldY * 16.0f;

			DrawTextureRec(
				game->Game->World.MainTileMap.TileSet->TileTexture,
				textureRect,
				worldPosition
				WHITE);
		}
	}
}

TileMapChunk* LoadChunk(ThreadedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk chunk{};
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