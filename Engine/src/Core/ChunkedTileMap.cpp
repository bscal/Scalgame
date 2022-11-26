#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Structures/SLinkedList.h"

#include <assert.h>

namespace ChunkedTileMap
{

// TODO
global_var SRandom Random;

internal uint64_t ChunkMapHash(const ChunkCoord* key)
{
	return std::hash<Vector2i>{}(*key);
}

internal bool ChunkMapEquals(const ChunkCoord* lhs, const ChunkCoord* rhs)
{
	return lhs->Equals(*rhs);
}

void Initialize(ChunkedTileMap* tilemap, TileSet* tileSet, Vector2i mapStartPos,
	Vector2i mapEndPos, Vector2i chunkDimensions)
{
	if (!tilemap)
	{
		S_LOG_ERR("ChunkedTileMap: tilemap cannot be nullptr");
		return;
	}
	if (!tileSet)
	{
		S_LOG_ERR("ChunkedTileMap: tileSet cannot be nullptr");
		return;
	}
	if (mapStartPos.x > mapEndPos.x)
	{
		S_LOG_ERR("ChunkedTileMap: mapStartPos.x cannot be > mapEndPos.x");
		return;
	}
	if (mapStartPos.y > mapEndPos.y)
	{
		S_LOG_ERR("ChunkedTileMap: mapStartPos.y cannot be > mapEndPos.y");
		return;
	}
	if (chunkDimensions.x <= 0 || chunkDimensions.y <= 0)
	{
		S_LOG_ERR("ChunkedTileMap: chunkDimensions must be > than 0");
		return;
	}

	SRandomInitialize(&Random, 0);

	tilemap->TileSet = tileSet;
	tilemap->StartPos = mapStartPos;
	tilemap->EndPos = mapEndPos;
	tilemap->ChunkDimensionsInTiles = chunkDimensions;
	tilemap->ChunkTileSize = (size_t)chunkDimensions.x * (size_t)chunkDimensions.y;
	tilemap->WorldDimensionsInTiles =
	{
		(tilemap->EndPos.x - tilemap->StartPos.x) * tilemap->ChunkDimensionsInTiles.x,
		(tilemap->EndPos.y - tilemap->StartPos.y) * tilemap->ChunkDimensionsInTiles.y,
	};
	tilemap->ViewDistance.x = 4.0f;
	tilemap->ViewDistance.y = 3.0f;
	size_t capacity = (size_t)(tilemap->ViewDistance.x * tilemap->ViewDistance.y);
	tilemap->ChunksList.InitializeCap(capacity);
	size_t mapCapacity = (size_t)((float)capacity + (float)capacity * .5f);
	tilemap->ChunksMap.InitializeEx(mapCapacity, &ChunkMapHash, &ChunkMapEquals);
}

void Free(ChunkedTileMap* tilemap)
{
	if (!tilemap)
	{
		S_LOG_ERR("ThreadedTileMapFree: tilemap cannot be nullptr");
		return;
	}
	assert(tilemap->ChunksList.IsInitialized());
	assert(tilemap->ChunksMap.IsInitialized());
	tilemap->ChunksList.Free();
	tilemap->ChunksMap.Free();
	SLinkedFree(&tilemap->ChunksToUnload);
}

void Create(ChunkedTileMap* tilemap, int loadWidth,
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

void FindChunksInView(ChunkedTileMap* tilemap, Game* game)
{
	Player player = game->Player;

	ChunkCoord chunkCoord = GetWorldTileToChunkCoord(tilemap,
		player.TilePosition.x, player.TilePosition.y);

	int startX = tilemap->StartPos.x;
	int endX = tilemap->EndPos.x;
	int startY = tilemap->StartPos.x;
	int endY = tilemap->EndPos.y;
	for (int chunkY = startY; chunkY < endY; ++chunkY)
	{
		for (int chunkX = startX; chunkX < endX; ++chunkX)
		{
			ChunkCoord nextChunkCoord = { chunkX, chunkY };
			if (nextChunkCoord.IsInBounds(
				tilemap->StartPos, tilemap->EndPos)) continue;
			if (IsChunkLoaded(tilemap, nextChunkCoord)) continue;
			// TODO think its safe to do this
			const auto chunk = LoadChunk(tilemap, nextChunkCoord);
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
			}
			TraceLog(LOG_INFO, "Chunk Loaded: X: %d, Y: %d "
				"WasGenerated: %d, Total Chunks: %d",
				chunkX, chunkY, chunk->IsChunkGenerated, tilemap->ChunksList.Length);
		}
	}
}

void Update(ChunkedTileMap* tilemap, GameApplication* gameApp)
{
	assert(tilemap);
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		UpdateChunk(tilemap, tilemap->ChunksList.PeekAtPtr(i), gameApp);
	}
	FindChunksInView(tilemap, gameApp->Game);
}

void UpdateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk, GameApplication* gameApp)
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
			textureRect.x = (float)tile.TextureCoord.x;
			textureRect.y = (float)tile.TextureCoord.y;
			textureRect.width = (float)tile.TextureCoord.w;
			textureRect.height = (float)tile.TextureCoord.h;

			float worldX = (float)chunk->ChunkCoord.x *
				((float)tilemap->ChunkDimensionsInTiles.x * 16.0f);
			float worldY = (float)chunk->ChunkCoord.y *
				((float)tilemap->ChunkDimensionsInTiles.y * 16.0f);
			Vector2 worldPosition;
			worldPosition.x = worldX + (float)x * 16.0f;
			worldPosition.y = worldY + (float)y * 16.0f;

			DrawTextureRec(
				tilemap->TileSet->TileTexture,
				textureRect,
				worldPosition,
				WHITE);
		}
	}
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk chunk = {};
	chunk.Tiles.InitializeCap(tilemap->ChunkTileSize);
	chunk.ChunkCoord = coord;

	tilemap->ChunksList.Push(&chunk);
	TileMapChunk* chunkPtrInChunksList = tilemap->ChunksList.Last();
	tilemap->ChunksMap.Put(&coord, &chunkPtrInChunksList);
	return chunkPtrInChunksList;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&coord);
	if (!chunk) return;

	tilemap->ChunksMap.Remove(&coord);

	// TODO maybe look into storing index in Chunk, and swapping
	// with last chunk here?
	for (size_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		if (tilemap->ChunksList[i].ChunkCoord.Equals(coord))
		{
			tilemap->ChunksList.RemoveAtFast(i);
		}
	}
}

void GenerateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk)
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
			uint32_t tileId = (uint32_t)SRandNextRange(&Random, 3, 6);
			TileMapTile tile = {};
			tile.TileId = tileId;
			uint16_t texX = (tileId % 16) * 16;
			uint16_t texY = (tileId / 16) * 16;
			tile.TextureCoord.x = texX;
			tile.TextureCoord.y = texY;
			tile.TextureCoord.w = 16;
			tile.TextureCoord.h = 16;
			tile.FowLevel = 0;
			chunk->Tiles[index++] = tile;
		}
	}
}

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	return tilemap->ChunksMap.Contains(&coord);
}

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	return *tilemap->ChunksMap.Get(&coord);
}

ChunkCoord GetWorldTileToChunkCoord(ChunkedTileMap* tilemap,
	int tileX, int tileY)
{
	ChunkCoord result;
	result.x = tileX / tilemap->ChunkDimensionsInTiles.x;
	result.y = tileY / tilemap->ChunkDimensionsInTiles.y;
	return result;
}

size_t GetWorldTileToChunkIndex(ChunkedTileMap* tilemap,
	int tileX, int tileY)
{
	int tileChunkX = tileX % tilemap->ChunkDimensionsInTiles.x;
	int tileChunkY = tileY % tilemap->ChunkDimensionsInTiles.y;
	return tileChunkX + tileChunkY * tilemap->ChunkDimensionsInTiles.x;
}

void SetTile(ChunkedTileMap* tilemap, TileMapTile* tile,
	int tileX, int tileY)
{
	ChunkCoord chunkCoord = GetWorldTileToChunkCoord(tilemap, tileX, tileY);
	uint32_t tileChunkCoord = GetWorldTileToChunkIndex(tilemap, tileX, tileY);
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

TileMapTile* GetTile(ChunkedTileMap* tilemap,
	int tileX, int tileY)
{
	ChunkCoord chunkCoord = GetWorldTileToChunkCoord(tilemap, tileX, tileY);
	uint32_t tileChunkCoord = GetWorldTileToChunkIndex(tilemap, tileX, tileY);
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

}
