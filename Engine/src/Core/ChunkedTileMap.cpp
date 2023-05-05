#include "ChunkedTileMap.h"

#include "Globals.h"
#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Vector2i.h"
#include "Renderer.h"

#include "Structures/SLinkedList.h"

#include "raylib/src/raymath.h"

namespace CTileMap
{

internal void CheckChunksInLOS(ChunkedTileMap* tilemap, Vector2i curChunkCoord);
internal void UpdateTileMap(ChunkedTileMap* tilemap, TileMapRenderer* tilemapRenderer);

internal const char*
ChunkStateToString(ChunkState state)
{
	constexpr const char* States[] = { "Unloaded", "Loaded", "Sleep" };
	SASSERT((uint8_t)state < (uint8_t)ChunkState::MaxStates);
	return States[(uint8_t)state];
}

void Initialize(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap);

	tilemap->ViewDistance.x = VIEW_DISTANCE.x;
	tilemap->ViewDistance.y = VIEW_DISTANCE.y;

	tilemap->WorldDimChunks = { 4, 4 };

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x * CHUNK_DIMENSIONS;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y * CHUNK_DIMENSIONS;

	SASSERT(tilemap->ViewDistance.x > 0);
	SASSERT(tilemap->ViewDistance.y > 0);
	uint32_t padding = 1u;
	uint32_t capacity = (padding + tilemap->ViewDistance.x * 2u) * (padding + tilemap->ViewDistance.y * 2u);
	SASSERT(capacity > 0);
	tilemap->Chunks.Reserve(capacity);
}

void Free(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap->Chunks.IsAllocated());

	tilemap->Chunks.Free();
	tilemap->ChunksToUnload.Free();
}

void Load(ChunkedTileMap* tilemap)
{
	CheckChunksInLOS(tilemap, { 0, 0 });
}

void Update(ChunkedTileMap* tilemap, Game* game)
{
	PROFILE_BEGIN();
	const PlayerEntity* player = GetClientPlayer();

	Vector2i playerTilePos = player->TilePos;
	Vector2i playerChunkPos = TileToChunkCoord(playerTilePos);

	// View distance checks + chunk loading
	if (player->HasMoved)
	{
		CheckChunksInLOS(tilemap, playerChunkPos);
	}
	
	UpdateTileMap(tilemap, &game->TileMapRenderer);

	constexpr float viewDistanceSqr = ((float)VIEW_DISTANCE.x + 1.0f) * ((float)VIEW_DISTANCE.x + 1.0f);
	for (uint32_t i = 0; i < tilemap->Chunks.Size; ++i)
	{
		auto& chunk = tilemap->Chunks[i];
		if (!chunk.Occupied) continue;

		float dist = Vector2DistanceSqr(playerChunkPos.AsVec2(), chunk.Value.ChunkCoord.AsVec2());

		// TODO: revisit this, current chunks dont need to be updated outside
		// view distance, but they might, or to handle rebuilds?
		if (dist > viewDistanceSqr)
		{
			tilemap->ChunksToUnload.Push(&chunk.Value.ChunkCoord);
		}
		else
		{
			GetGameApp()->NumOfChunksUpdated++;
			UpdateChunk(tilemap, &chunk.Value);
		}
	}

	while (tilemap->ChunksToUnload.HasNext())
	{
		Vector2i chunkCoord = tilemap->ChunksToUnload.PopValue();
		UnloadChunk(tilemap, chunkCoord);
	}

	PROFILE_END();
}

void LateUpdate(ChunkedTileMap* tilemap, Game* game)
{
	if (game->DebugTileView)
	{
		Rectangle screen;
		screen.x = GetGameApp()->ScreenXY.x;
		screen.y = GetGameApp()->ScreenXY.y;
		screen.width = (float)GetScreenWidth();
		screen.height = (float)GetScreenHeight();
		for (uint32_t i = 0; i < tilemap->Chunks.Size; ++i)
		{
			auto& chunk = tilemap->Chunks[i];
			if (!chunk.Occupied) continue;
			if (CheckCollisionRecs(screen, chunk.Value.Bounds))
			{
				DrawRectangleLinesEx(chunk.Value.Bounds, 4, GREEN);
			}
		}
	}
}

void 
UpdateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk)
{
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	if (!IsChunkInBounds(tilemap, coord)) return nullptr;
	if (IsChunkLoaded(tilemap, coord)) return nullptr;

	TileMapChunk* chunk = tilemap->Chunks.InsertKey(&coord);
	SASSERT(chunk);
	chunk->ChunkCoord = coord;

	constexpr float chunkDimensionsPixel = (float)CHUNK_DIMENSIONS * TILE_SIZE_F;
	chunk->Bounds.x = (float)coord.x * chunkDimensionsPixel;
	chunk->Bounds.y = (float)coord.y * chunkDimensionsPixel;
	chunk->Bounds.width = chunkDimensionsPixel;
	chunk->Bounds.height = chunkDimensionsPixel;

	MapGenGenerateChunk(&GetGame()->MapGen, tilemap, chunk);

	chunk->State = ChunkState::Loaded;

	SLOG_INFO("[ Chunk ] Loaded chunk (%s). State: %s", FMT_VEC2I(coord), ChunkStateToString(chunk->State));
	return chunk;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	bool removed = tilemap->Chunks.Remove(&coord);
	if (removed)
		SLOG_INFO("[ Chunk ] Unloaded chunk (%s)", FMT_VEC2I(coord));
}

bool IsChunkLoaded(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	// TODO
	return (GetChunk(tilemap, coord) != nullptr);
}

bool IsTileInBounds(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	return (tilePos.x >= -tilemap->WorldDimTiles.x
		&& tilePos.y >= -tilemap->WorldDimTiles.y
		&& tilePos.x < tilemap->WorldDimTiles.x
		&& tilePos.y < tilemap->WorldDimTiles.y);
}

bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos)
{
	return (chunkPos.x >= -tilemap->WorldDimChunks.x
		&& chunkPos.y >= -tilemap->WorldDimChunks.y
		&& chunkPos.x < tilemap->WorldDimChunks.x
		&& chunkPos.y < tilemap->WorldDimChunks.y);
}

TileMapChunk* 
GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	return tilemap->Chunks.Get(&coord);
}

TileMapChunk* 
GetChunkByTile(ChunkedTileMap* tilemap, TileCoord tileCoord)
{
	ChunkCoord coord = TileToChunkCoord(tileCoord);
	return GetChunk(tilemap, coord);
}

ChunkCoord 
TileToChunkCoord(TileCoord tilePos)
{
	ChunkCoord result;
	result.x = (int)floorf((float)tilePos.x / (float)CHUNK_DIMENSIONS);
	result.y = (int)floorf((float)tilePos.y / (float)CHUNK_DIMENSIONS);
	return result;
}

size_t 
TileToIndex(TileCoord tilePos)
{
	int tileChunkX = IModNegative(tilePos.x, CHUNK_DIMENSIONS);
	int tileChunkY = IModNegative(tilePos.y, CHUNK_DIMENSIONS);
	size_t result = (size_t)tileChunkX + (size_t)tileChunkY * CHUNK_DIMENSIONS;
	SASSERT(result < CHUNK_SIZE);
	return result;
}

void 
SetTile(ChunkedTileMap* tilemap, const TileData* tile, TileCoord tilePos)
{
	SASSERT(tilemap);
	SASSERT(tile);
	SASSERT(tilemap->Chunks.IsAllocated());
	SASSERT(IsTileInBounds(tilemap, tilePos));

	ChunkCoord chunkCoord = TileToChunkCoord(tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);

	// TODO: Not sure how to handle.
	// probably should do runtime check? should chunk then be loaded?
	// or just ignored?
	if (!chunk || chunk->State == ChunkState::Unloaded)
	{
		SLOG_WARN("[ Tilemap ] GET failed! tile(%s), nonexistent chunk(%s)",
			FMT_VEC2I(tilePos), FMT_VEC2I(chunkCoord));
		return;
	}
	uint64_t index = TileToIndex(tilePos);
	chunk->Tiles[index] = *tile;
}

TileData* 
GetTile(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	SASSERT(tilemap);
	SASSERT(tilemap->Chunks.IsAllocated());
	SASSERT(IsTileInBounds(tilemap, tilePos));

	ChunkCoord chunkCoord = TileToChunkCoord(tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk || chunk->State == ChunkState::Unloaded)
	{
		SLOG_WARN("[ Tilemap ] GET failed! tile(%s), nonexistent chunk(%s)",
			FMT_VEC2I(tilePos), FMT_VEC2I(chunkCoord));
		return nullptr;
	}
	uint64_t index = TileToIndex(tilePos);
	return &chunk->Tiles[index];
}

TileCoord WorldToTile(Vector2 pos)
{
	Vector2i v;
	v.x = (int)(floorf(pos.x / TILE_SIZE_F));
	v.y = (int)(floorf(pos.y / TILE_SIZE_F));
	return v;
}

// TODO: this works fine, but im not quite sure where i want to store
// Line of sight status for tiles. It doesnt really need to be persistent?
// but we need to store it currently for lighting. LOS in TileData is currently
// unused.
void SetVisible(ChunkedTileMap* tilemap, TileCoord coord)
{
	if (!IsTileInBounds(tilemap, coord)) return;
	Vector2i cullTile = WorldTileToCullTile(coord);
	int index = cullTile.x + cullTile.y * CULL_WIDTH_TILES;
	GetGame()->TileMapRenderer.Tiles[index].LOS = true;
	//GetTile(tilemap, coord)->LOS = TileLOS::FullVision;
}

bool BlocksLight(ChunkedTileMap* tilemap, TileCoord coord)
{
	if (!IsTileInBounds(tilemap, coord)) return true;
	TileData* tileData = GetTile(tilemap, coord);
	SASSERT(tileData);
	return tileData->GetTile()->Type == TileType::Solid;
}

internal void
CheckChunksInLOS(ChunkedTileMap* tilemap, Vector2i chunkCoord)
{
	PROFILE_BEGIN();
	Vector2i start = chunkCoord.Subtract(tilemap->ViewDistance);
	Vector2i end = chunkCoord.Add(tilemap->ViewDistance);
	for (int chunkY = start.y; chunkY <= end.y; ++chunkY)
	{
		for (int chunkX = start.x; chunkX <= end.x; ++chunkX)
		{
			LoadChunk(tilemap, { chunkX, chunkY });
		}
	}
	PROFILE_END();
}

internal void
UpdateTileMap(ChunkedTileMap* tilemap, TileMapRenderer* tilemapRenderer)
{
	PROFILE_BEGIN();
	size_t idx = 0;
	for (int y = 0; y < CULL_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < CULL_WIDTH_TILES; ++x)
		{
			Vector2i coord = CullTileToWorldTile({ x, y });

			if (IsTileInBounds(tilemap, coord))
			{
				TileData* tileData = GetTile(tilemap, coord);
				tilemapRenderer->Tiles[idx].x = tileData->TexX;
				tilemapRenderer->Tiles[idx].y = tileData->TexY;
				tilemapRenderer->Tiles[idx].HasCeiling = tileData->HasCeiling;
				// See SetVisible()
				if (GetGame()->DebugDisableDarkess)
					tilemapRenderer->Tiles[idx].LOS = 1;

				Tile* tile = tileData->GetTile();
				if (tile->OnUpdate)
					tile->OnUpdate(coord, *tileData);
			}
			else
			{
				tilemapRenderer->Tiles[idx] = {};
			}

			++idx;
		}
	}
	PROFILE_END();
}

}
