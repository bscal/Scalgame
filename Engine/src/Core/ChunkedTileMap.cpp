#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "Structures/SLinkedList.h"

#include <assert.h>

namespace ChunkedTileMap
{

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

void Initialize(ChunkedTileMap* tilemap, TileSet* tileSet,
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
	tilemap->TileSet = tileSet;
	tilemap->TileMapDimensionsInChunks = tileMapDimensionsInChunks;
	tilemap->ChunkDimensionsInTiles = chunkDimensionsInTiles;
	tilemap->ChunkSize = chunkDimensionsInTiles.x * chunkDimensionsInTiles.y;
	tilemap->ChunksList.Initialize();
	tilemap->ChunksMap.InitializeEx(16, &ChunkMapHash, &ChunkMapEquals);
	tilemap->ViewDistanceInChunk.x = 4;
	tilemap->ViewDistanceInChunk.y = 3;
}

void Free(ChunkedTileMap* tilemap)
{
	if (!tilemap)
	{
		TraceLog(LOG_ERROR, "ThreadedTileMapFree: tilemap cannot be nullptr");
		return;
	}

	tilemap->ChunksList.Free();
	tilemap->ChunksMap.Free();
	SLinkedFree<ChunkCoord>(&tilemap->ChunksToUnload);
	SLinkedFree<ChunkCoord>(&tilemap->ChunksToLoad);
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

	int startX = chunkCoord.x - tilemap->ViewDistanceInChunk.x;
	int endX = chunkCoord.x + tilemap->ViewDistanceInChunk.x;
	int startY = chunkCoord.y - tilemap->ViewDistanceInChunk.y;
	int endY = chunkCoord.y + tilemap->ViewDistanceInChunk.y;
	for (int chunkY = startY; chunkY < endY; ++chunkY)
	{
		for (int chunkX = startX; chunkX < endX; ++chunkX)
		{
			uint32_t x = static_cast<uint32_t>(chunkX);
			uint32_t y = static_cast<uint32_t>(chunkY);
			ChunkCoord nextChunkCoord = { x, y };
			if (!IsChunkInBounds(tilemap, x, y)) continue;
			if (IsChunkLoaded(tilemap, nextChunkCoord)) continue;
			// TODO think its safe to do this
			const auto chunk = LoadChunk(tilemap, nextChunkCoord);
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
			}
			TraceLog(LOG_INFO, "Chunk Loaded: X: %d, Y: %d "
				"WasGenerated: %d, Total Chunks: %d",
				x, y, chunk->IsChunkGenerated, tilemap->ChunksList.Length);
			//SLinkedListPush(&tilemap->ChunksToLoad, &nextChunkCoord);
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
	chunk.Tiles.InitializeCap(tilemap->ChunkSize);
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
		if (tilemap->ChunksList[i].ChunkCoord.AreEquals(coord))
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
	uint64_t tileX, uint64_t tileY)
{
	uint64_t chunkX = tileX / (uint64_t)tilemap->TileMapDimensionsInChunks.x;
	uint64_t chunkY = tileY / (uint64_t)tilemap->TileMapDimensionsInChunks.y;
	return { (uint32_t)chunkX, (uint32_t)chunkY };
}

uint32_t GetWorldTileToChunkIndex(ChunkedTileMap* tilemap,
	uint64_t tileX, uint64_t tileY)
{
	uint64_t tileChunkX = tileX % (uint64_t)tilemap->ChunkDimensionsInTiles.x;
	uint64_t tileChunkY = tileY % (uint64_t)tilemap->ChunkDimensionsInTiles.y;
	return tileChunkX + tileChunkY * tilemap->ChunkDimensionsInTiles.x;
}

void SetTile(ChunkedTileMap* tilemap, TileMapTile* tile,
	uint64_t tileX, uint64_t tileY)
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
	uint64_t tileX, uint64_t tileY)
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

bool IsChunkInBounds(ChunkedTileMap* tilemap, uint32_t chunkX, uint32_t chunkY)
{
	return chunkX < tilemap->TileMapDimensionsInChunks.x &&
		chunkY < tilemap->TileMapDimensionsInChunks.y;
}

}
