#include "ChunkedTileMap.h"

#include "Globals.h"
#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Vector2i.h"
#include "Renderer.h"

#include "Structures/SLinkedList.h"

#include "raymath.h"

namespace CTileMap
{

internal void CheckChunksInLOS(ChunkedTileMap* tilemap);
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
	SASSERT(!tilemap->ChunksList.IsAllocated());

	tilemap->ViewDistance.x = VIEW_DISTANCE.x;
	tilemap->ViewDistance.y = VIEW_DISTANCE.y;

	tilemap->WorldDimChunks = { 4, 4 };

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x * CHUNK_DIMENSIONS;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y * CHUNK_DIMENSIONS;

	SASSERT(tilemap->ViewDistance.x > 0);
	SASSERT(tilemap->ViewDistance.y > 0);
	uint32_t capacity = (3u + tilemap->ViewDistance.x) * (3u + tilemap->ViewDistance.y) + 3u;
	SASSERT(capacity > 0);
	tilemap->ChunksList.Reserve(capacity);
}

void Free(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap->ChunksList.IsAllocated());

	tilemap->ChunksList.Free();
	tilemap->ChunksToUnload.Free();
}

void Load(ChunkedTileMap* tilemap)
{
	CheckChunksInLOS(tilemap);
}

void Update(ChunkedTileMap* tilemap, Game* game)
{
	PROFILE_BEGIN();
	const Player* player = GetClientPlayer();

	Vector2i playerChunkCoord = TileToChunkCoord(player->Transform.TilePos);

	// View distance checks + chunk loading
	if (player->HasMoved)
	{
		CheckChunksInLOS(tilemap);
	}
	
	UpdateTileMap(tilemap, &game->TileMapRenderer);

	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
		Vector2i difference = playerChunkCoord.Subtract(chunk->ChunkCoord);
		int chunkX = labs(difference.x);
		int chunkY = labs(difference.y);

		// TODO: revisit this, current chunks dont need to be updated outside
		// view distance, but they might, or to handle rebuilds?
		if (chunkX > VIEW_DISTANCE.x + 1 || chunkY > VIEW_DISTANCE.y + 1)
		{
			tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
		}
		else
		{
			GetGameApp()->NumOfChunksUpdated++;
			//UpdateChunk(tilemap, chunk, game);
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
		for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
		{
			TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
			if (CheckCollisionRecs(screen, chunk->Bounds))
			{
				DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
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
	if (IsChunkLoaded(tilemap, coord)) return nullptr;

	TileMapChunk* chunk = tilemap->ChunksList.PushNew();
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
	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		if (tilemap->ChunksList[i].ChunkCoord.Equals(coord))
		{
			tilemap->ChunksList.RemoveAtFast(i);
			tilemap->ChunksList[tilemap->ChunksList.Count].State = ChunkState::Unloaded;
			tilemap->ChunksList[tilemap->ChunksList.Count].ChunkCoord = { -1, -1 };
			SLOG_INFO("[ Chunk ] Unloaded chunk (%s)", FMT_VEC2I(coord));
			break;
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
	return (tilePos.x >= 0 
		&& tilePos.y >= 0
		&& tilePos.x < tilemap->WorldDimTiles.x
		&& tilePos.y < tilemap->WorldDimTiles.y);
}

bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos)
{
	return (chunkPos.x >= 0
		&& chunkPos.y >= 0
		&& chunkPos.x < tilemap->WorldDimChunks.x
		&& chunkPos.y < tilemap->WorldDimChunks.y);
}

TileMapChunk* 
GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
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
	ChunkCoord coord = TileToChunkCoord(tileCoord);
	for (size_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		const auto& chunk = tilemap->ChunksList[i];
		if (chunk.ChunkCoord.Equals(coord))
			return &tilemap->ChunksList.Memory[i];
	}
	return nullptr;
}

ChunkCoord 
TileToChunkCoord(TileCoord tilePos)
{
	ChunkCoord result;
	result.x = tilePos.x / CHUNK_DIMENSIONS;
	result.y = tilePos.y / CHUNK_DIMENSIONS;
	return result;
}

uint64_t 
TileToIndex(TileCoord tilePos)
{
	int tileChunkX = tilePos.x % CHUNK_DIMENSIONS;
	int tileChunkY = tilePos.y % CHUNK_DIMENSIONS;
	return static_cast<uint64_t>(tileChunkX + tileChunkY * CHUNK_DIMENSIONS);
}

void 
SetTile(ChunkedTileMap* tilemap, const TileData* tile, TileCoord tilePos)
{
	SASSERT(tilemap);
	SASSERT(tile);
	SASSERT(tilemap->ChunksList.IsAllocated());
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
	SASSERT(tilemap->ChunksList.IsAllocated());
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
	v.x = (int)(floorf(pos.x) / TILE_SIZE_F);
	v.y = (int)(floorf(pos.y) / TILE_SIZE_F);
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
	return IsTileInBounds(tilemap, coord) && GetTile(tilemap, coord)->GetTile()->Type == TileType::Solid;
}

internal void
CheckChunksInLOS(ChunkedTileMap* tilemap)
{
	PROFILE_BEGIN();
	Vector2i target = GetClientPlayer()->Transform.TilePos;
	Vector2i start = target.Subtract(tilemap->ViewDistance);
	Vector2i end = target.Add(tilemap->ViewDistance);
	for (int chunkY = start.y; chunkY <= end.y; ++chunkY)
	{
		for (int chunkX = start.x; chunkX <= end.x; ++chunkX)
		{
			ChunkCoord nextChunkCoord = { chunkX, chunkY };
			if (!IsChunkInBounds(tilemap, nextChunkCoord)) continue;
			LoadChunk(tilemap, nextChunkCoord);
		}
	}
	PROFILE_END();
}

internal void
UpdateTileMap(ChunkedTileMap* tilemap, TileMapRenderer* tilemapRenderer)
{
	Vector2i offset = GetGameApp()->CullXYTiles;
	for (int y = 0; y < CULL_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < CULL_WIDTH_TILES; ++x)
		{
			// Add here because CullTileToWorldTile
			TileCoord coord;
			coord.x = x + offset.x;
			coord.y = y + offset.y;

			size_t index = x + y * CULL_WIDTH_TILES;
			
			if (IsTileInBounds(tilemap, coord))
			{
				TileData* tile = GetTile(tilemap, coord);
				tilemapRenderer->Tiles[index].x = tile->TexX;
				tilemapRenderer->Tiles[index].y = tile->TexY;
				tilemapRenderer->Tiles[index].HasCeiling = tile->HasCeiling;
				// See SetVisible()
				if (GetGame()->DebugDisableDarkess)
					tilemapRenderer->Tiles[index].LOS = 1;
			}
			else
			{
				tilemapRenderer->Tiles[index] = {};
			}
		}
	}	
}

}
