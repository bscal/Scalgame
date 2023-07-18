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
	constexpr uint32_t capacity = 5 * 5 * 2;
	static_assert(capacity > 0, "capactiy > 0");
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
	//CheckChunksInLOS(tilemap, { 0, 0 });
}

void Update(ChunkedTileMap* tilemap, Game* game)
{
	SASSERT(tilemap);
	SASSERT(game);

	PROFILE_BEGIN();

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
		if (tilemap->Chunks.Buckets[i].Occupied)
		{
			TileMapChunk* chunk = tilemap->Chunks.Buckets[i].Value;

			GetGameApp()->NumOfChunksUpdated += 1;

			constexpr float viewDistance = VIEW_DISTANCE * VIEW_DISTANCE;
			constexpr float viewDistanceSqr = viewDistance * viewDistance;
			float dist = Vector2DistanceSqr(playerChunkPos.AsVec2(), chunk->ChunkCoord.AsVec2());

			// TODO: revisit this, current chunks dont need to be updated outside
			// view distance, but they might, or to handle rebuilds?
			if (dist > viewDistanceSqr)
			{
				tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
			}
			else
			{
				// Chunk Update
				if (FlagTrue(chunk->RebakeFlags, CHUNK_REBAKE_SELF))
				{
					BakeChunkLighting(tilemap, chunk, FlagTrue(chunk->RebakeFlags, CHUNK_REBAKE_NEIGHBORS));
				}

				chunk->TileUpdater.Update(tilemap, chunk, GetDeltaTime());
			}
		}
	}

	while (tilemap->ChunksToUnload.HasNext())
	{
		Vector2i chunkCoord = tilemap->ChunksToUnload.PopValue();
		UnloadChunk(tilemap, chunkCoord);
	}

	UpdateTileMap(tilemap, &game->TileMapRenderer);

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

		tilemap->Chunks.Foreach([&screen](TileMapChunk** chunkPtr)
		{
			TileMapChunk* chunk = *chunkPtr;
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

	TileMapChunk* chunk = (TileMapChunk*)SAlloc(SAllocator::Game, sizeof(TileMapChunk), MemoryTag::Game);
	SASSERT(chunk);
	tilemap->Chunks.Insert(&coord, &chunk);

	SMemClear(chunk, sizeof(TileMapChunk));
	chunk->ChunkCoord = coord;

	constexpr float chunkDimensionsPixel = (float)CHUNK_DIMENSIONS * TILE_SIZE_F;

	chunk->StartTile.x = coord.x * CHUNK_DIMENSIONS;
	chunk->StartTile.y = coord.y * CHUNK_DIMENSIONS;

	chunk->Bounds.x = (float)coord.x * chunkDimensionsPixel;
	chunk->Bounds.y = (float)coord.y * chunkDimensionsPixel;
	chunk->Bounds.width = chunkDimensionsPixel;
	chunk->Bounds.height = chunkDimensionsPixel;

	chunk->RebakeFlags = CHUNK_REBAKE_ALL;

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

			++idx;
		}
	}

	chunk->State = ChunkState::Loaded;

	SLOG_INFO("[ Chunk ] Loaded chunk (%s). State: %s", FMT_VEC2I(coord), ChunkStateToString(chunk->State));
	return chunk;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk** chunkPtr = tilemap->Chunks.Get(&coord);
	if (chunkPtr)
	{
		TileMapChunk* chunk = *chunkPtr;
		SASSERT(chunk);

		tilemap->Chunks.Remove(&coord);

		SFree(SAllocator::Game, chunk, sizeof(TileMapChunk), MemoryTag::Game);

		SLOG_INFO("[ Chunk ] Unloaded chunk (%s)", FMT_VEC2I(coord));
	}
}

void BakeChunkLighting(ChunkedTileMap* tilemap, TileMapChunk* chunk, int chunkBakeFlags)
{
	SASSERT(tilemap);
	SASSERT(chunk);
	SASSERT(IsChunkLoaded(tilemap, chunk->ChunkCoord));

	if (chunk->IsBaked)
	{
		SMemClear(chunk->TileColors.Data, chunk->TileColors.MemorySize());
	}

	chunk->IsBaked = true;
	chunk->RebakeFlags = 0;

	Rectangle tileBounds;
	tileBounds.x = (float)chunk->StartTile.x;
	tileBounds.y = (float)chunk->StartTile.y;
	tileBounds.width = CHUNK_DIMENSIONS;
	tileBounds.height = CHUNK_DIMENSIONS;

	// Bakes all lights inside current chunk and applies only
	// to the current chunk.
	{
		int idx = 0;
		for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
		{
			for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
			{
				Vector2i coord = chunk->StartTile + Vector2i{ x, y };
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
					StaticLightDrawToChunk(&light, chunk, tilemap);
				}

				++idx;
			}
		}
	}

	// Bakes all lights for surrounding chunks and applies to only
	// the current chunk. If using bakeFlag CHUNK_REBAKE_NEIGHBORS those
	// neighbors will be marked to update themselves. A rectangle
	// collision check is used to check if a light should be baked or not.
	for (int i = 0; i < ArrayLength(Vec2i_NEIGHTBORS_CORNERS); ++i)
	{
		Vector2i neighborCoords = chunk->ChunkCoord + Vec2i_NEIGHTBORS_CORNERS[i];
		TileMapChunk* neighborChunk = GetChunk(tilemap, neighborCoords);
		if (neighborChunk)
		{
			if (FlagTrue(chunkBakeFlags, CHUNK_REBAKE_NEIGHBORS))
			{
				neighborChunk->RebakeFlags = CHUNK_REBAKE_SELF;
			}

			int idx = 0;
			for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
			{
				for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
				{
					Vector2i coord = neighborChunk->StartTile + Vector2i{ x, y };
					TileData* data = &neighborChunk->Tiles[idx];
					Tile* tile = data->GetTile();

					if (tile->EmitsLight)
					{
						StaticLight light;
						light.Color = RED;
						light.LightType = LightType::Static;
						light.Pos = coord;
						light.Radius = 2;
						light.StaticLightType = StaticLightTypes::Basic;
						light.UpdateFunc = nullptr;

						Rectangle lightBounds;
						lightBounds.x = coord.x - light.Radius;
						lightBounds.y = coord.y - light.Radius;
						lightBounds.width = light.Radius * 2 + 1;
						lightBounds.height = lightBounds.width;

						if (CheckCollisionRecs(tileBounds, lightBounds))
						{
							StaticLightDrawToChunk(&light, chunk, tilemap);
						}
					}
					++idx;
				}
			}
		}
	}
}

bool IsChunkLoaded(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	// TODO
	return (tilemap->Chunks.Get(&coord));
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
	TileMapChunk** chunkPtr = tilemap->Chunks.Get(&coord);
	return (chunkPtr) ? *chunkPtr : nullptr;
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
	//GetGame()->TileMapRenderer.Tiles[index].LOS = true;
	GetGame()->LightingRenderer.TileData[index].r = 1;
	//TileLighDataLOSSet(GetGame()->LightingRenderer.TileData[index], 1);
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
	LightingRenderer* lightRenderer = &GetGame()->LightingRenderer;

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

				tilemapRenderer->Tiles[idx].x = tileData->TexX;
				tilemapRenderer->Tiles[idx].y = tileData->TexY;

				lightRenderer->TileData[idx].g = (uint8_t)tileData->HasCeiling;
				//TileLighDataCeilingSet(lightRenderer->TileData[idx], (uint8_t)tileData->HasCeiling);

				SMemCopy(&lightRenderer->TileColors[idx], tileColor, sizeof(Color));

				// See SetVisible()
				if (GetGame()->DebugDisableDarkess)
					lightRenderer->TileData[idx].r = 1;
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
