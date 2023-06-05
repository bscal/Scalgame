#include "TileMap.h"

#include "Game.h"

#include <math.h>

_FORCE_INLINE_ internal int
IModNegative(int a, int b)
{
	int res = a % b;
	return (res < 0) ? res + b : res;
}

void TileMap::Initialize()
{
	ChunksToUnload.Allocator = SAllocator::Temp;

	ViewDistanceInChunks = { 4, 4 };
	WorldDimensionsInChunks = { 20, 20 };
}

void TileMap::Free()
{
	Chunks.Free();
	ChunksToUnload.Free();
}

void TileMap::UpdateChunks(Game* game)
{

}

void TileMap::CullAndDrawChunks(Game* game, RenderTexture2D target)
{
	Rectangle viewport;
	viewport.x = game->ViewCamera.target.x;
	viewport.y = game->ViewCamera.target.y;
	viewport.width = (float)GetScreenWidth();
	viewport.height = (float)GetScreenHeight();
	
	BeginTextureMode(target);

	for (uint32_t i = 0; i < Chunks.Capacity; ++i)
	{
		SHoodBucket<Vector2i, TileChunk>& bucket = Chunks.Buckets[i];
		if (bucket.Occupied && bucket.Value.IsLoaded)
		{
			bool shouldDraw = CheckCollisionRecs(viewport, bucket.Value.BoundingBox);
			if (shouldDraw)
			{
				Rectangle src = { 0, 0, TileChunk::CHUNK_DIMENSION * TILE_SIZE_F, TileChunk::CHUNK_DIMENSION * TILE_SIZE_F };
				Rectangle dst = bucket.Value.BoundingBox;
				DrawTexturePro(bucket.Value.RenderTexture.texture, src, dst, {}, 0.0f, WHITE);
			}
		}
	}

	EndTextureMode();
}

TileChunk* TileMap::LoadChunk(Vector2i coord)
{
	TileChunk* chunk = Chunks.InsertKey(&coord);
	SASSERT(chunk);

	if (!chunk->IsLoaded)
	{
		SASSERT(chunk->RenderTexture.texture.id == 0);

		SMemClear(chunk, sizeof(TileChunk));

		int chunkSize = CHUNK_DIMENSIONS * TILE_SIZE;

		chunk->RenderTexture = LoadRenderTexture(chunkSize, chunkSize);

		chunk->BoundingBox.x = (float)coord.x * TILE_SIZE_F;
		chunk->BoundingBox.y = (float)coord.x * TILE_SIZE_F;
		chunk->BoundingBox.width = (float)chunkSize;
		chunk->BoundingBox.height = (float)chunkSize;
	}

	chunk->IsLoaded = true;
	return chunk;
}

void TileMap::UnloadChunk(Vector2i coord)
{
	TileChunk* chunk = Chunks.InsertKey(&coord);
	SASSERT(chunk);

	if (chunk->IsLoaded)
	{
		if (chunk->RenderTexture.texture.id != 0)
		{
			UnloadRenderTexture(chunk->RenderTexture);
		}
	}

	if (IsRenderTextureReady(chunk->RenderTexture))
	{
		size_t size = ArrayLength(chunk->TilesMarkedForRebuild.Memory);
		SMemClear(chunk->TilesMarkedForRebuild.Memory, size);
		BuildChunk(chunk);
	}
	else
	{
		SASSERT_MSG(false, "RenderTexture is not ready!");
	}

	chunk->IsLoaded = false;
}

void BuildChunk(TileChunk* chunk)
{
	int chunkSize = CHUNK_DIMENSIONS * TILE_SIZE;

	if (chunk->RenderTexture.texture.id != 0)
	{
		for (size_t i = 0; i < TileChunk::CHUNK_SIZE; ++i)
		{
			if (chunk->TilesMarkedForRebuild.Get(i))
			{
				int x = i % TileChunk::CHUNK_DIMENSION;
				int y = i / TileChunk::CHUNK_DIMENSION;
				
				Rectangle src = {};
				Rectangle dst = { chunk->BoundingBox.x + (float)x, chunk->BoundingBox.y + (float)y, TILE_SIZE_F, TILE_SIZE_F};
				DrawTexturePro(chunk->RenderTexture.texture, src, dst, {}, 0.0f, WHITE);
			}
		}
	}
}

TileInfo* TileMap::GetTile(Vector2i tile)
{
	Vector2i chunkCoord = GetTileChunkCoord(tile);
	TileChunk* chunk = Chunks.Get(&chunkCoord);

	if (!chunk || !chunk->IsLoaded)
	{
		SLOG_WARN("Trying to GET tile from unloaded chunk (%s)", FMT_VEC2I(chunkCoord));
		return nullptr;
	}

	uint32_t idx = GetTileIndex(tile);
	return &chunk->Tiles[idx];
}

void TileMap::SetTile(Vector2i tile, const TileInfo* data, ChunkFlags flags)
{
	Vector2i chunkCoord = GetTileChunkCoord(tile);
	TileChunk* chunk = Chunks.Get(&chunkCoord);

	if (!chunk || !chunk->IsLoaded)
	{
		SLOG_WARN("Trying to SET tile from unloaded chunk (%s)", FMT_VEC2I(chunkCoord));
		return;
	}

	uint32_t idx = GetTileIndex(tile);
	TileInfo* tileData = &chunk->Tiles[idx];
	SMemCopy(tileData, data, sizeof(TileInfo));

	if (flags != 0)
	{
		chunk->ShouldRebuild = true;
		chunk->TilesMarkedForRebuild.Set(idx);
	}
}

Vector2i TileMap::GetTileChunkCoord(Vector2i tile)
{
	Vector2i chunkCoord;
	chunkCoord.x = (int)std::floorf((float)tile.x / (float)CHUNK_DIMENSIONS);
	chunkCoord.y = (int)std::floorf((float)tile.y / (float)CHUNK_DIMENSIONS);
	return chunkCoord;
}

uint32_t TileMap::GetTileIndex(Vector2i tile)
{
	int x = IModNegative(tile.x, CHUNK_DIMENSIONS);
	int y = IModNegative(tile.y, CHUNK_DIMENSIONS);
	int idx = x + y * CHUNK_DIMENSIONS;
	SASSERT(idx >= 0);
	return idx;
}