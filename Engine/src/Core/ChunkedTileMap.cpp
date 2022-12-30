#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"

#include "Structures/SLinkedList.h"

#include "raymath.h"

#include <cassert>

namespace CTileMap
{

// TODO
static SRandom Random;

internal uint64_t ChunkMapHash(const ChunkCoord* key)
{
	return std::hash<Vector2i>{}(*key);
}

internal bool ChunkMapEquals(const ChunkCoord* lhs, const ChunkCoord* rhs)
{
	return lhs->Equals(*rhs);
}

void Initialize(ChunkedTileMap* tilemap, Game* game)
{
	assert(tilemap);
	assert(!tilemap->ChunksList.IsInitialized(), "Cannot Initialize already initialized tilemap");

	SRandomInitialize(&Random, 0);

	Vector2i origin = { 0, 0 };
	Vector2i worldDim = { 4, 4 };

	tilemap->TileSize = { TILE_SIZE, TILE_SIZE };
	tilemap->ChunkSize = { CHUNK_SIZE, CHUNK_SIZE };
	tilemap->Origin = origin;
	tilemap->WorldDimChunks = worldDim;

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x
		* tilemap->ChunkSize.x;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y
		* tilemap->ChunkSize.y;

	tilemap->ViewDistance.x = game->ChunkViewDistance.x;
	tilemap->ViewDistance.y = game->ChunkViewDistance.y;

	tilemap->ChunkTileCount = tilemap->ChunkSize.x *
		tilemap->ChunkSize.y;

	tilemap->WorldBounds.x = (float)(tilemap->Origin.x * tilemap->TileSize.x);
	tilemap->WorldBounds.y = (float)(tilemap->Origin.y * tilemap->TileSize.y);
	tilemap->WorldBounds.width = (float)(tilemap->WorldDimChunks.x
		* tilemap->ChunkSize.x * tilemap->TileSize.x);
	tilemap->WorldBounds.height = (float)(tilemap->WorldDimChunks.y
		* tilemap->ChunkSize.y * tilemap->TileSize.y);

	tilemap->ChunkBounds.x = 0.0f;
	tilemap->ChunkBounds.y = 0.0f;
	tilemap->ChunkBounds.width = (float)(tilemap->ChunkSize.x
		* tilemap->TileSize.x);
	tilemap->ChunkBounds.height = (float)(tilemap->ChunkSize.y
		* tilemap->TileSize.y);

	size_t capacity = (size_t)(tilemap->ViewDistance.x * tilemap->ViewDistance.y);
	tilemap->ChunksList.InitializeCap(capacity);

	assert(tilemap->Origin.x >= 0 && tilemap->Origin.y >= 0, "Non 0, 0 world origin is current not supported");
}

void Free(ChunkedTileMap* tilemap)
{
	assert(tilemap->ChunksList.IsInitialized(), "TileMap not initialized");
	tilemap->ChunksList.Free();
	tilemap->ChunksToUnload.Free();
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

void FindChunksInView(ChunkedTileMap* tilemap,
	Game* game)
{
	const Player* player = GetClientPlayer();
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, player->Transform.TilePos);
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
			const auto chunk = LoadChunk(tilemap, nextChunkCoord);
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
			}
		}
	}
}

internal void FindChunksInViewDistance(ChunkedTileMap* tilemap, Game* game, ChunkCoord playerChunkCoord)
{
	int startX = playerChunkCoord.x - (int)tilemap->ViewDistance.x;
	int endX = playerChunkCoord.x + (int)tilemap->ViewDistance.x;
	int startY = playerChunkCoord.y - (int)tilemap->ViewDistance.y;
	int endY = playerChunkCoord.y + (int)tilemap->ViewDistance.y;
	for (int chunkY = startY; chunkY < endY; ++chunkY)
	{
		for (int chunkX = startX; chunkX < endX; ++chunkX)
		{
			ChunkCoord chunkCoord = { chunkX, chunkY };
			if (!IsChunkInBounds(tilemap, chunkCoord)) continue;
			if (IsChunkLoaded(tilemap, chunkCoord)) continue;

			auto chunk = LoadChunk(tilemap, chunkCoord);
			S_LOG_INFO("[ Chunk ] Chunk loaded (x: %d, y: %d)", chunkX, chunkY);
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
				S_LOG_INFO("[ Chunk ] Chunk generated (x: %d, y: %d)", chunkX, chunkY);
			}
		}
	}
}

internal void UpdateChunks(ChunkedTileMap* tilemap, Game* game, ChunkCoord playerChunkCoord)
{
	Vector2 offset = Vector2Subtract(game->WorldCamera.target, game->WorldCamera.offset);
	Rectangle screenRect
	{
		offset.x, offset.y,
		(float)GetScreenWidth(), (float)GetScreenHeight()
	};
	for (uint16_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(screenRect, chunk->Bounds))
		{
			UpdateChunk(tilemap, chunk, game);
		}
		else
		{
			constexpr float viewDistance = VIEW_DISTANCE.x * VIEW_DISTANCE.x;
			float dist = chunk->ChunkCoord.Distance(playerChunkCoord);
			if (dist > viewDistance)
			{
				tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
				S_LOG_INFO("[ Chunk ] Chunk marked for removal (x: %d, y: %d)",
					chunk->ChunkCoord.x, chunk->ChunkCoord.y);
			}
		}
	}
}

void Update(ChunkedTileMap* tilemap, Game* game)
{
	const Player* player = GetClientPlayer();
	ChunkCoord pChunkCoord = TileToChunkCoord(tilemap, player->Transform.TilePos);
	if (player->HasMoved)
	{
		FindChunksInViewDistance(tilemap, game, pChunkCoord);
	}

	UpdateChunks(tilemap, game, pChunkCoord);
	Draw(tilemap, game);
}



internal void Draw(ChunkedTileMap* tilemap, Game* game)
{
	const auto& texture = game->Atlas.Texture;
	Vector2i tileSize = tilemap->TileSize;
	Vector2i screenCoord = GetClientPlayer()->Transform.TilePos;
	Vector2i screenDims
	{
		GetRenderWidth() / tileSize.x,
		GetRenderHeight() / tileSize.y
	};
	float offsetX = (game->WorldCamera.offset.x) / (float)tileSize.x;
	float offsetY = (game->WorldCamera.offset.y) / (float)tileSize.y;
	int tileOffsetX = (int)((float)screenCoord.x - offsetX);
	int tileOffsetY = (int)((float)screenCoord.y - offsetY);
	for (int y = 0; y < screenDims.y; ++y)
	{
		for (int x = 0; x < screenDims.x; ++x)
		{
			TileCoord coord = {
				x + tileOffsetX,
				y + tileOffsetY
			};
			if (!IsTileInBounds(tilemap, coord)) continue;

			Tile* tile = GetTile(tilemap, coord);
			assert(tile);

			if (tile->LOS == TileLOS::NoVision) continue;

			Vector2 position
			{
				(float)(coord.x * tileSize.x),
				(float)(coord.y * tileSize.y)
			};
			DrawTextureRec(
				texture,
				tile->TextureRect,
				position,
				tile->TileColor);

			tile->LOS = TileLOS::NoVision;
		}
	}
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
}

void UpdateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk, Game* game)
{
	assert(chunk->IsChunkGenerated);

	GetGameApp()->NumOfChunksUpdated++;

	//const auto& texture = game->Atlas.Texture;

	//float incrementX = (float)tilemap->TileSize.x;
	//float incrementY = (float)tilemap->TileSize.y;
	//float chunkX = chunk->Bounds.x;
	//float chunkY = chunk->Bounds.y;
	//int i = 0;
	//for (float y = 0.f; y < chunk->Bounds.height; y += incrementX)
	//{
	//	for (float x = 0.f; x < chunk->Bounds.width; x += incrementY)
	//	{
	//		const auto& tile = chunk->Tiles[i++];
	//		game->World.LightMap.UpdateTile(&game->World,
	//			{ (int)(chunkX + x) / 16, (int)(chunkY + y) / 16 },
	//			&tile);
	//	}
	//}
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

bool IsTileInBounds(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	return (tilePos.x >= tilemap->Origin.x &&
		tilePos.y >= tilemap->Origin.y &&
		tilePos.x < tilemap->WorldDimTiles.x&&
		tilePos.y < tilemap->WorldDimTiles.y);
}

bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos)
{
	return (chunkPos.x >= tilemap->Origin.x &&
		chunkPos.y >= tilemap->Origin.y &&
		chunkPos.x < tilemap->WorldDimChunks.x&&
		chunkPos.y < tilemap->WorldDimChunks.y);
}

TileMapChunk* GetChunk(
	ChunkedTileMap* tilemap, ChunkCoord coord)
{
	for (size_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		const auto& chunk = tilemap->ChunksList[i];
		if (chunk.ChunkCoord.Equals(coord))
			return &tilemap->ChunksList.Memory[i];
	}
	return nullptr;
}

ChunkCoord TileToChunkCoord(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	ChunkCoord result;
	result.x = tilePos.x / CHUNK_SIZE;
	result.y = tilePos.y / CHUNK_SIZE;
	return result;
}

uint64_t TileToIndex(ChunkedTileMap* tilemap, ChunkCoord chunkCoord,
	TileCoord tilePos)
{
	int tileChunkX = tilePos.x % CHUNK_SIZE;
	int tileChunkY = tilePos.y % CHUNK_SIZE;
	return static_cast<uint64_t>(tileChunkX + tileChunkY * tilemap->ChunkSize.x);
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
	uint64_t index = TileToIndex(tilemap, chunkCoord, tilePos);
	chunk->Tiles.Set(index, tile);
}

Tile* GetTile(ChunkedTileMap* tilemap, TileCoord tilePos)
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
	uint64_t index = TileToIndex(tilemap, chunkCoord, tilePos);
	return chunk->Tiles.PeekAtPtr(index);
}

const Tile& GetTileRef(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	assert(chunk);
	assert(chunk->IsChunkGenerated);
	// TODO should be always assert if chunk doesnt exist/not generated
	uint64_t index = TileToIndex(tilemap, chunkCoord, tilePos);
	assert(index < tilemap->ChunkTileCount);
	return chunk->Tiles[index];
}

TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos)
{
	Vector2i v;
	v.x = (int)pos.x / tilemap->TileSize.x;
	v.y = (int)pos.y / tilemap->TileSize.y;
	return v;
}

void SetVisible(ChunkedTileMap* tilemap, TileCoord coord)
{
	if (!IsTileInBounds(tilemap, coord)) return;
	GetTile(tilemap, coord)->LOS = TileLOS::FullVision;
}

bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord)
{
	if (!IsTileInBounds(tilemap, coord)) return true;

	Tile* tile = GetTile(tilemap, coord);
	assert(tile);
	return tile->GetTileData(&GetGameApp()->Game->World.TileMgr).Type == TileType::Solid;
}

}