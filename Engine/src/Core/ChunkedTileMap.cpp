#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Structures/SLinkedList.h"

#include <assert.h>

namespace CTileMap
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

void Initialize(ChunkedTileMap* tilemap,
	Vector2i tileSize, Vector2i origin,
	Vector2i worldDimChunks, Vector2i chunkSize)
{
	if (!tilemap)
	{
		S_LOG_ERR("ChunkedTileMap: tilemap cannot be nullptr");
		return;
	}
	if (origin.x > worldDimChunks.x)
	{
		S_LOG_ERR("ChunkedTileMap: mapStartPos.x cannot be > mapEndPos.x");
		return;
	}
	if (origin.y > worldDimChunks.y)
	{
		S_LOG_ERR("ChunkedTileMap: mapStartPos.y cannot be > mapEndPos.y");
		return;
	}
	if (chunkSize.x <= 0 || chunkSize.y <= 0)
	{
		S_LOG_ERR("ChunkedTileMap: chunkDimensions must be > than 0");
		return;
	}

	SRandomInitialize(&Random, 0);

	tilemap->TileSize = tileSize;
	tilemap->Origin = origin;
	tilemap->OriginTiles.x =
		(float)(tilemap->Origin.x * tilemap->ChunkSize.x);
	tilemap->OriginTiles.y =
		(float)(tilemap->Origin.y * tilemap->ChunkSize.y);
	tilemap->WorldDimChunks = worldDimChunks;
	tilemap->ChunkSize = chunkSize;

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x 
		* tilemap->ChunkSize.x;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y
		* tilemap->ChunkSize.y;

	tilemap->ViewDistance.x = 3.0f;
	tilemap->ViewDistance.y = 2.0f;

	tilemap->ChunkTileCount = tilemap->ChunkSize.x *
		tilemap->ChunkSize.y;

	tilemap->WorldBounds.x = tilemap->OriginTiles.x * tilemap->TileSize.x;
	tilemap->WorldBounds.y = tilemap->OriginTiles.x * tilemap->TileSize.y;
	tilemap->WorldBounds.width =
		tilemap->WorldDimChunks.x
		* tilemap->ChunkSize.x
		* tilemap->TileSize.x;
	tilemap->WorldBounds.height =
		tilemap->WorldDimChunks.y
		* tilemap->ChunkSize.y
		* tilemap->TileSize.y;

	tilemap->ChunkBounds.x = 0.0f;
	tilemap->ChunkBounds.y = 0.0f;
	tilemap->ChunkBounds.width = tilemap->ChunkSize.x
		* tilemap->TileSize.x;
	tilemap->ChunkBounds.height = tilemap->ChunkSize.y
		* tilemap->TileSize.y;

	size_t capacity = (size_t)(tilemap->ViewDistance.x * tilemap->ViewDistance.y);
	tilemap->ChunksList.InitializeCap(capacity);
}

void Free(ChunkedTileMap* tilemap)
{
	if (!tilemap)
	{
		S_LOG_ERR("ThreadedTileMapFree: tilemap cannot be nullptr");
		return;
	}
	assert(tilemap->ChunksList.IsInitialized());
	tilemap->ChunksList.Free();
	SLinkedFree(&tilemap->ChunksToUnload);
}

internal void CreateChunk(ChunkedTileMap* tilemap, int loadWidth,
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

internal bool Distance(ChunkCoord coord, int w, int h)
{
	Vector2i o = { w, h };
	const auto dist = coord.Distance(o);
	return (dist > 3);
}

void FindChunksInView(ChunkedTileMap* tilemap,
	Game* game)
{
	auto& player = game->World.EntityMgr.Players[0];

	ChunkCoord chunkCoord = TileToChunkCoord(tilemap,
		player.Transform.TilePos);

	int startX = chunkCoord.x - (int)tilemap->ViewDistance.x;
	int endX = chunkCoord.x + (int)tilemap->ViewDistance.x;
	int startY = chunkCoord.y - (int)tilemap->ViewDistance.y;
	int endY = chunkCoord.y + (int)tilemap->ViewDistance.y;
	for (int chunkY = startY; chunkY < endY; ++chunkY)
	{
		for (int chunkX = startX; chunkX < endX; ++chunkX)
		{
			ChunkCoord nextChunkCoord = { chunkX, chunkY };
			if (!IsChunkInBounds(tilemap, nextChunkCoord)) continue;
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

void Update(ChunkedTileMap* tilemap, Game* game)
{
	assert(tilemap);
	assert(game);
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(game->CurScreenRect, chunk->Bounds))
		{
			UpdateChunk(tilemap, chunk, game);
		}
	}
	FindChunksInView(tilemap, game);
}

void LateUpdateChunk(ChunkedTileMap* tilemap, Game* game)
{
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(game->CurScreenRect, chunk->Bounds))
		{
			DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
		}
	}
	DrawRectangleLinesEx(game->CurScreenRect, 4, SKYBLUE);
}

void UpdateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk, Game* game)
{
	assert(chunk->IsChunkGenerated);

	GetGameApp()->NumOfChunksUpdated++;

	const auto& texture = game->Atlas.Texture;

	float incrementX = (float)tilemap->TileSize.x;
	float incrementY = (float)tilemap->TileSize.y;
	float chunkX = chunk->Bounds.x;
	float chunkY = chunk->Bounds.y;
	int i = 0;
	for (float y = 0.f; y < chunk->Bounds.height; y += incrementX)
	{
		for (float x = 0.f; x < chunk->Bounds.width; x += incrementY)
		{
			const auto& tile = chunk->Tiles[i++];

			game->World.LightMap.UpdateTile(&game->World,
				{ (int)(chunkX + x) / 16, (int)(chunkY + y) / 16 },
				&tile);

			Vector2 worldPos = { chunkX + x, chunkY + y };
			DrawTextureRec(
				texture,
				tile.TextureRect,
				worldPos,
				tile.TileColor);
		}
	}
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	TileMapChunk chunk = {};
	chunk.Tiles.InitializeCap(tilemap->ChunkTileCount);
	chunk.ChunkCoord = coord;

	float chunkOffSetX = (float)coord.x * (float)tilemap->TileSize.x
		* (float)tilemap->ChunkSize.x;
	float chunkOffSetY = (float)coord.y * (float)tilemap->TileSize.y
		* (float)tilemap->ChunkSize.y;

	chunk.Bounds = tilemap->ChunkBounds;
	chunk.Bounds.x += chunkOffSetX;
	chunk.Bounds.y += chunkOffSetY;

	tilemap->ChunksList.Push(&chunk);
	return tilemap->ChunksList.Last();
}

void UnloadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	for (size_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		if (tilemap->ChunksList[i].ChunkCoord.Equals(coord))
		{
			tilemap->ChunksList.RemoveAtFast(i);
		}
	}
}

void GenerateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk)
{
	assert(chunk);
	if (chunk->IsChunkGenerated)
	{
		S_LOG_WARN("Trying to generate an already generated chunk!");
		return;
	}
	chunk->IsChunkGenerated = true;

	for (int y = 0; y < tilemap->ChunkSize.y; ++y)
	{
		for (int x = 0; x < tilemap->ChunkSize.x; ++x)
		{
			uint32_t tileId = (uint32_t)SRandNextRange(&Random, 0, 2);
			const auto& tile = CreateTileId(
				&GetGameApp()->Game->World.TileMgr,
				tileId);
			chunk->Tiles.Push(&tile);
		}
	}
}

bool IsChunkLoaded(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	// TODO
	return (GetChunk(tilemap, coord) != nullptr);
}

bool IsTileInBounds(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	return (tilePos.x >= tilemap->OriginTiles.x &&
		tilePos.y >= tilemap->OriginTiles.y &&
		tilePos.x < tilemap->WorldDimTiles.x &&
		tilePos.y < tilemap->WorldDimTiles.y);
}

bool IsChunkInBounds(ChunkedTileMap* tilemap,
	ChunkCoord chunkPos)
{
	return (chunkPos.x >= tilemap->Origin.x &&
		chunkPos.y >= tilemap->Origin.y &&
		chunkPos.x < tilemap->WorldDimChunks.x &&
		chunkPos.y < tilemap->WorldDimChunks.y);
}

TileMapChunk* GetChunk(
	ChunkedTileMap* tilemap, ChunkCoord coord)
{
	for (size_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		const auto chunkPtr = tilemap->ChunksList.PeekAtPtr(i);
		if (chunkPtr->ChunkCoord.Equals(coord))
			return chunkPtr;
	}
	return nullptr;
}

ChunkCoord TileToChunkCoord(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	ChunkCoord result;
	result.x = tilePos.x / tilemap->ChunkSize.x;
	result.y = tilePos.y / tilemap->ChunkSize.y;
	return result;
}

uint64_t TileToIndex(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	int x = tilePos.x - tilemap->Origin.x;
	int y = tilePos.y - tilemap->Origin.y;
	assert(x >= 0);
	assert(y >= 0);
	int tileChunkX = x % tilemap->ChunkSize.x;
	int tileChunkY = y % tilemap->ChunkSize.y;
	return static_cast<uint64_t>(
		tileChunkX + tileChunkY * tilemap->ChunkSize.x);
}

uint64_t TileToIndexLocal(Vector2i origin, uint64_t width,
	TileCoord tilePos)
{
	Vector2i localPos = tilePos.Subtract(origin);
	return (uint64_t)localPos.x + (uint64_t)localPos.y * width;
}

void SetTile(ChunkedTileMap* tilemap,
	const Tile* tile, TileCoord tilePos)
{
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk)
	{
		S_LOG_WARN("SETTING tile[%d, %d] to "
			"unloaded or nonexistent chunk[%d, %d]",
			tilePos.x, tilePos.y, chunkCoord.x, chunkCoord.y);
		return;
	}
	uint64_t index = TileToIndex(tilemap, tilePos);
	chunk->Tiles.Set(index, tile);
}

Tile* GetTile(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk)
	{
		S_LOG_WARN("GETTING tile[%d, %d] to "
			"unloaded or nonexistent chunk[%d, %d]",
			tilePos.x, tilePos.y, chunkCoord.x, chunkCoord.y);
		return nullptr;
	}

	uint64_t index = TileToIndex(tilemap, tilePos);
	return chunk->Tiles.PeekAtPtr(index);
}

TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos)
{
	Vector2i v;
	v.x = (int)pos.x / tilemap->TileSize.x;
	v.y = (int)pos.y / tilemap->TileSize.y;
	return v;
}

}