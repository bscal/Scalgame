#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Vector2i.h"

#include "Structures/SLinkedList.h"

#include "raymath.h"

#include <cassert>

namespace CTileMap
{

// TODO
static SRandom Random;

void Initialize(ChunkedTileMap* tilemap, Game* game)
{
	SASSERT(tilemap);
	SASSERT(!tilemap->ChunksList.IsInitialized());

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

	SASSERT_MSG(tilemap->Origin.x >= 0 && tilemap->Origin.y >= 0, "Non 0, 0 world origin is current not supported");
}

void Free(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap->ChunksList.IsInitialized());
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
			TileMapChunk* chunk = LoadChunk(tilemap, nextChunkCoord);
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
			SLOG_INFO("[ Chunk ] Chunk loaded (x: %d, y: %d)", chunkX, chunkY);
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
				SLOG_INFO("[ Chunk ] Chunk generated (x: %d, y: %d)", chunkX, chunkY);
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
	for (uint16_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(screenRect, chunk->Bounds))
		{
			UpdateChunk(tilemap, chunk, game);
		}
		else
		{
			constexpr float viewDistance = (float)(VIEW_DISTANCE.x * VIEW_DISTANCE.y);
			float dist = chunk->ChunkCoord.Distance(playerChunkCoord);
			if (dist > viewDistance)
			{
				tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
				SLOG_INFO("[ Chunk ] Chunk marked for removal (x: %d, y: %d)",
					chunk->ChunkCoord.x, chunk->ChunkCoord.y);
			}
		}
	}
}

void Update(ChunkedTileMap* tilemap, Game* game)
{
	const Player* player = GetClientPlayer();
	ChunkCoord pChunkCoord = player->Transform.ChunkPos;
	if (player->HasMoved)
	{
		FindChunksInViewDistance(tilemap, game, pChunkCoord);
	}

	Draw(tilemap, game);
	UpdateChunks(tilemap, game, pChunkCoord);
}

internal void Draw(ChunkedTileMap* tilemap, Game* game)
{
	const auto& tileTexture = game->Resources.Atlas.Texture;
	TileMgr* tileMgr = &game->TileMgr;

	const int PADDING = 1;
	const int WH_PADDING = 2;
	Vector2i target = GetClientPlayer()->Transform.TilePos;
	Vector2i screenDims
	{
		GetScreenWidth() / TILE_SIZE + PADDING + WH_PADDING,
		GetScreenHeight() / TILE_SIZE + PADDING + WH_PADDING
	};
	float offsetX = game->WorldCamera.offset.x / TILE_SIZE;
	float offsetY = game->WorldCamera.offset.y / TILE_SIZE;
	int tileOffsetX = (int)((float)target.x - offsetX - PADDING);
	int tileOffsetY = (int)((float)target.y - offsetY - PADDING);
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
			SASSERT(tile);

			Rectangle position
			{
				(float)(coord.x * TILE_SIZE),
				(float)(coord.y * TILE_SIZE),
				(float)TILE_SIZE,
				(float)TILE_SIZE
			};
			if (tile->LOS == TileLOS::FullVision || game->DebugDisableFOV)
			{
				DrawTextureProF(
					tileTexture,
					tile->GetTileTexData(tileMgr).TexCoord,
					position,
					{},
					0.0f,
					tile->Color);
			}
			tile->LOS = TileLOS::NoVision;

			if (game->DebugTileView)
				DrawRectangleLinesEx(position, 1.0f, PINK);

			if (game->DebugDisableDarkess)
				tile->Color = { 1.0f, 1.0f, 1.0f, 1.0f };
			else
				tile->Color = { 1.0f, 1.0f, 1.0f, 0.0f };
		}
	}
}

void LateUpdateChunk(ChunkedTileMap* tilemap, Game* game)
{
	if (!game->DebugTileView) return; 
	const auto& screenRect = GetScaledScreenRect();

	for (uint64_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(screenRect, chunk->Bounds))
		{
			DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
		}
	}
}

void UpdateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk, Game* game)
{
	GetGameApp()->NumOfChunksUpdated++;

	// NOTE: it is move convenient to clear here. Needs
	// to be reset somewhere, not sure if there is a 
	// better solution
	Scal::MemClear(chunk->LastLightPos, sizeof(chunk->LastLightPos));
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	TileMapChunk* chunk = tilemap->ChunksList.PushZero();
	//chunk.Tiles.InitializeCap(tilemap->ChunkTileCount);
	chunk->Entities.InitializeCap(10); // TODO
	chunk->ChunkCoord = coord;

	float chunkOffSetX = (float)coord.x * (float)tilemap->TileSize.x
		* (float)tilemap->ChunkSize.x;
	float chunkOffSetY = (float)coord.y * (float)tilemap->TileSize.y
		* (float)tilemap->ChunkSize.y;

	chunk->Bounds = tilemap->ChunkBounds;
	chunk->Bounds.x += chunkOffSetX;
	chunk->Bounds.y += chunkOffSetY;
	return chunk;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	for (size_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		auto chunkPtr = tilemap->ChunksList.PeekAtPtr(i);
		if (chunkPtr->ChunkCoord.Equals(coord))
		{
			//chunkPtr->Tiles.Free();
			chunkPtr->Entities.Free();
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
		SLOG_WARN("Trying to generate an already generated chunk!");
		return;
	}
	chunk->IsChunkGenerated = true;

	for (int y = 0; y < tilemap->ChunkSize.y; ++y)
	{
		for (int x = 0; x < tilemap->ChunkSize.x; ++x)
		{
			uint32_t tileId = (uint32_t)SRandNextRange(&Random, 1, 1);
			const auto& tile = CreateTileId(
				&GetGameApp()->Game->TileMgr,
				tileId);
			chunk->Tiles[x + y * CHUNK_SIZE] = tile;
			//chunk->Tiles.Push(&tile);
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
	for (size_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		const auto& chunk = tilemap->ChunksList[i];
		if (chunk.ChunkCoord.Equals(coord))
			return &tilemap->ChunksList.Memory[i];
	}
	return nullptr;
}

TileMapChunk*
GetChunkByTile(ChunkedTileMap* tilemap, TileCoord tileCoord)
{
	ChunkCoord coord = TileToChunkCoord(tilemap, tileCoord);
	for (size_t i = 0; i < tilemap->ChunksList.Count; ++i)
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

uint64_t TileToIndex(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	int tileChunkX = tilePos.x % CHUNK_SIZE;
	int tileChunkY = tilePos.y % CHUNK_SIZE;
	return static_cast<uint64_t>(tileChunkX + tileChunkY * tilemap->ChunkSize.x);
}

void SetTile(ChunkedTileMap* tilemap,
	const Tile* tile, TileCoord tilePos)
{
	assert(tilemap->ChunksList.IsInitialized());
	assert(IsTileInBounds(tilemap, tilePos));
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk)
	{
		SLOG_WARN("SETTING tile[%d, %d] to "
			"unloaded or nonexistent chunk[%d, %d]",
			tilePos.x, tilePos.y, chunkCoord.x, chunkCoord.y);
		return;
	}
	assert(chunk->IsChunkGenerated);
	uint64_t index = TileToIndex(tilemap, tilePos);
	chunk->Tiles[index] = *tile;
	//chunk->Tiles.Set(index, tile);
}

Tile* GetTile(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	assert(tilemap->ChunksList.IsInitialized());
	assert(IsTileInBounds(tilemap, tilePos));
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk)
	{
		SLOG_WARN("GETTING tile[%d, %d] to "
			"unloaded or nonexistent chunk[%d, %d]",
			tilePos.x, tilePos.y, chunkCoord.x, chunkCoord.y);
		return nullptr;
	}
	assert(chunk->IsChunkGenerated);
	uint64_t index = TileToIndex(tilemap, tilePos);
	return &chunk->Tiles[index];
	//return chunk->Tiles.PeekAtPtr(index);
}

const Tile& GetTileRef(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	assert(chunk);
	assert(chunk->IsChunkGenerated);
	// TODO should be always assert if chunk doesnt exist/not generated
	uint64_t index = TileToIndex(tilemap, tilePos);
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
	return tile->GetTileData(&GetGameApp()->Game->TileMgr).Type == TileType::Solid;
}

}