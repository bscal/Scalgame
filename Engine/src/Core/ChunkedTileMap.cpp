#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Vector2i.h"
#include "Renderer.h"

#include "Structures/SLinkedList.h"

#include "raylib/src/raymath.h"

namespace CTileMap
{

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

	tilemap->ViewDistance.x = VIEW_DISTANCE;
	tilemap->ViewDistance.y = VIEW_DISTANCE;

	tilemap->WorldDimChunks = { 4, 4 };

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x * CHUNK_DIMENSIONS;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y * CHUNK_DIMENSIONS;

	SASSERT(tilemap->ViewDistance.x > 0);
	SASSERT(tilemap->ViewDistance.y > 0);
	constexpr uint32_t capacity = VIEW_DISTANCE * VIEW_DISTANCE * VIEW_DISTANCE * VIEW_DISTANCE * 2;
	static_assert(capacity > 0, "capactiy > 0");
	tilemap->Chunks.Reserve(capacity);
	
	tilemap->TileUpdater.Interval = (float)(MAX_FPS / 4);
}

void Free(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap->Chunks.IsAllocated());

	tilemap->Chunks.Free();
	tilemap->ChunksToUnload.Free();
}

void Load(ChunkedTileMap* tilemap)
{
	//CheckChunksInLOS(tilemap, { 0, 0 });
}

void Update(ChunkedTileMap* tilemap, Game* game)
{
	SASSERT(tilemap);
	SASSERT(game);

	PROFILE_BEGIN();

	constexpr float viewDistance = VIEW_DISTANCE * VIEW_DISTANCE;
	constexpr float viewDistanceSqr = viewDistance * viewDistance;

	const Player* player = GetClientPlayer();

	//Vector2 playerPos = player->AsPosition();
	Vector2i playerChunkPos = TileToChunkCoord(player->TilePos);

	// View distance checks + chunk loading
	Vector2i start = playerChunkPos.Subtract(tilemap->ViewDistance);
	Vector2i end = playerChunkPos.Add(tilemap->ViewDistance);
	for (int chunkY = start.y; chunkY <= end.y; ++chunkY)
	{
		for (int chunkX = start.x; chunkX <= end.x; ++chunkX)
		{
			LoadChunk(tilemap, { chunkX, chunkY });
		}
	}
	
	for (uint32_t i = 0; i < tilemap->Chunks.Capacity; ++i)
	{
		const auto& chunk = tilemap->Chunks.Buckets[i];

		if (!chunk.Occupied)
			continue;

		++GetGameApp()->NumOfChunksUpdated;

		float dist = Vector2DistanceSqr(playerChunkPos.AsVec2(), chunk.Value.ChunkCoord.AsVec2());

		// TODO: revisit this, current chunks dont need to be updated outside
		// view distance, but they might, or to handle rebuilds?
		if (dist > viewDistanceSqr)
		{
			tilemap->ChunksToUnload.Push(&chunk.Value.ChunkCoord);
		}
	}

	while (tilemap->ChunksToUnload.HasNext())
	{
		Vector2i chunkCoord = tilemap->ChunksToUnload.PopValue();
		UnloadChunk(tilemap, chunkCoord);
	}

	UpdateTileMap(tilemap, &game->TileMapRenderer);

	tilemap->TileUpdater.Update(tilemap, GetDeltaTime());

	static float counter = 0;
	counter += GetDeltaTime();
	if (counter > 1.0)
	{
		counter = 0;
		SLOG_INFO("Updating %d tiles per frame. Interval: %f", tilemap->TileUpdater.UpdateCount, tilemap->TileUpdater.Interval);
	}

	PROFILE_END();
}

void LateUpdate(ChunkedTileMap* tilemap, Game* game)
{
	if (game->DebugTileView)
	{
		Rectangle screen;
		screen.x = GetGameApp()->View.ScreenXY.x;
		screen.y = GetGameApp()->View.ScreenXY.y;
		screen.width = (float)GetScreenWidth();
		screen.height = (float)GetScreenHeight();

		tilemap->Chunks.Foreach([&screen](TileMapChunk* chunk)
		{
			const char* chunkPosStr = TextFormat("%d, %d", chunk->ChunkCoord.x, chunk->ChunkCoord.y);
			DrawText(chunkPosStr, (int)chunk->Bounds.x, (int)chunk->Bounds.y, 32, WHITE);

			if (CheckCollisionRecs(screen, chunk->Bounds))
			{
				DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
			}
		});
	}
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	if (!IsChunkInBounds(tilemap, coord))
		return nullptr;

	if (IsChunkLoaded(tilemap, coord))
		return nullptr;

	TileMapChunk* chunk = tilemap->Chunks.InsertKey(&coord);
	SASSERT(chunk);
	SMemClear(chunk, sizeof(TileMapChunk));
	chunk->ChunkCoord = coord;

	constexpr float chunkDimensionsPixel = (float)CHUNK_DIMENSIONS * TILE_SIZE_F;
	constexpr float halfChunkDimensionsPixel = chunkDimensionsPixel / 2.0f;

	chunk->ChunkStartXY.x = coord.x * CHUNK_DIMENSIONS;
	chunk->ChunkStartXY.y = coord.y * CHUNK_DIMENSIONS;

	chunk->Bounds.x = (float)coord.x * chunkDimensionsPixel;
	chunk->Bounds.y = (float)coord.y * chunkDimensionsPixel;
	chunk->Bounds.width = chunkDimensionsPixel;
	chunk->Bounds.height = chunkDimensionsPixel;

	chunk->ChunkCenter.x = chunk->Bounds.x + halfChunkDimensionsPixel;
	chunk->ChunkCenter.y = chunk->Bounds.y + halfChunkDimensionsPixel;

	MapGenGenerateChunk(&GetGame()->MapGen, tilemap, chunk);

	int idx = 0;
	for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
	{
		for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
		{
			float worldX = (float)x + (float)chunk->ChunkCoord.x * (float)CHUNK_DIMENSIONS;
			float worldY = (float)y + (float)chunk->ChunkCoord.y * (float)CHUNK_DIMENSIONS;

			Vector2i coord = { (int)worldX, (int)worldY };
			TileData* data = &chunk->Tiles[idx];
			Tile* tile = data->GetTile();

			if (tile->OnUpdate)
			{
				chunk->TileUpdateIds[idx] = tilemap->TileUpdater.Add(coord, data);
			}

			++idx;
		}
	}

	BakeChunkLighting(tilemap, chunk, CHUNK_BAKE_FLAG_REBAKE_NEIGHBORS, 0);

	chunk->State = ChunkState::Loaded;

	SLOG_INFO("[ Chunk ] Loaded chunk (%s). State: %s", FMT_VEC2I(coord), ChunkStateToString(chunk->State));
	return chunk;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk* chunk = tilemap->Chunks.Get(&coord);
	if (chunk)
	{
		for (uint32_t i = 0; i < chunk->TileUpdateIds.Size(); ++i)
		{
			tilemap->TileUpdater.Actions.Remove(chunk->TileUpdateIds[i]);
		}

		tilemap->Chunks.Remove(&coord);
		SLOG_INFO("[ Chunk ] Unloaded chunk (%s)", FMT_VEC2I(coord));
	}
}

void BakeChunkLighting(ChunkedTileMap* tilemap, TileMapChunk* chunk, int chunkBakeFlags, int chunkSideFlags)
{
	SASSERT(tilemap);
	SASSERT(chunk);
	SASSERT(IsChunkLoaded(tilemap, chunk->ChunkCoord));

	int idx = 0;
	for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
	{
		for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
		{
			Vector2i coord = chunk->ChunkStartXY + Vector2i{ x, y };
			TileData* data = &chunk->Tiles[idx];
			Tile* tile = data->GetTile();

			if (tile->EmitsLight)
			{
				StaticLight light;
				light.Color = RED;
				light.LightType = LightType::Static;
				light.Pos = coord;
				light.Radius = 0;
				light.StaticLightType = StaticLightTypes::Basic;
				light.UpdateFunc = nullptr;
				StaticLightDrawToChunk(&light, chunk, tilemap, chunkSideFlags);
			}

			++idx;
		}
	}

	// If chunks around this chunk are already loaded before us,
	// rebake them so they update our values.
	if (chunkBakeFlags | CHUNK_BAKE_FLAG_REBAKE_NEIGHBORS)
	{
		for (int i = 0; i < ArrayLength(Vec2i_NEIGHTBORS_CORNERS); ++i)
		{
			Vector2i neighborCoords = chunk->ChunkCoord + Vec2i_NEIGHTBORS_CORNERS[i];
			TileMapChunk* neighborChunk = GetChunk(tilemap, neighborCoords);
			if (neighborChunk)
			{
				int sideFlags = GetNearSides(neighborCoords, chunk->ChunkCoord);
				BakeChunkLighting(tilemap, neighborChunk, 0, sideFlags);
			}
		}
	}
}

bool IsChunkLoaded(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	// TODO
	return tilemap->Chunks.Contains(&coord);
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
GetTileLocalIndex(TileCoord tilePos)
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
	uint64_t index = GetTileLocalIndex(tilePos);
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
	uint64_t index = GetTileLocalIndex(tilePos);
	return &chunk->Tiles[index];
}

TileCoord WorldToTile(Vector2 pos)
{
	Vector2i v;
	v.x = (int)floorf(pos.x / TILE_SIZE_F);
	v.y = (int)floorf(pos.y / TILE_SIZE_F);
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
	int index = cullTile.x + cullTile.y * GetGameApp()->View.ResolutionInTiles.x;
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
UpdateTileMap(ChunkedTileMap* tilemap, TileMapRenderer* tilemapRenderer)
{
	PROFILE_BEGIN();
	size_t idx = 0;
	for (int y = 0; y < GetGameApp()->View.ResolutionInTiles.y; ++y)
	{
		for (int x = 0; x < GetGameApp()->View.ResolutionInTiles.x; ++x)
		{
			Vector2i coord = CullTileToWorldTile({ x, y });

			if (IsTileInBounds(tilemap, coord))
			{
				TileMapChunk* chunk = GetChunkByTile(tilemap, coord);
				SASSERT(chunk);

				size_t localIdx = GetTileLocalIndex(coord);

				TileData* tileData = &chunk->Tiles[localIdx];
				Color* tileColor = &chunk->TileColors[localIdx];

				SMemCopy(&tilemapRenderer->Tiles[idx], tileData, 3ULL);

				constexpr float colorInverse = 1.f / 255.f;
				GetGame()->LightingRenderer.Tiles[idx].x = (float)tileColor->r * colorInverse;
				GetGame()->LightingRenderer.Tiles[idx].y = (float)tileColor->g * colorInverse;
				GetGame()->LightingRenderer.Tiles[idx].z = (float)tileColor->b * colorInverse;

				// See SetVisible()
				if (GetGame()->DebugDisableDarkess)
					tilemapRenderer->Tiles[idx].LOS = 1;
			}
			else
			{
				SMemSet(&tilemapRenderer->Tiles[idx], 0, sizeof(TileData));
				//SMemSet(&GetGame()->LightingRenderer.Tiles[idx], 0, sizeof(Color));
			}

			++idx;
		}
	}
	PROFILE_END();
}

}
