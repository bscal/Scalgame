#include "ChunkedTileMap.h"

#include "Globals.h"
#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Vector2i.h"
#include "Renderer.h"
#include "LightMap.h"

#include "Structures/SLinkedList.h"

#include "raymath.h"

namespace CTileMap
{

internal void CheckChunksInLOS(ChunkedTileMap* tilemap);
internal void Draw(ChunkedTileMap* tilemap, Game* game);

void Initialize(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap);
	SASSERT(!tilemap->ChunksList.IsAllocated());

	tilemap->ViewDistance.x = VIEW_DISTANCE.x;
	tilemap->ViewDistance.y = VIEW_DISTANCE.y;

	tilemap->Origin = { 0, 0 };
	tilemap->WorldDimChunks = { 4, 4 };

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x * CHUNK_DIMENSIONS;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y * CHUNK_DIMENSIONS;

	uint32_t capacity = (3 + tilemap->ViewDistance.x) * (3 + tilemap->ViewDistance.y) + 3;
	SASSERT(capacity > 0);
	tilemap->ChunksList.Reserve(capacity);

	SASSERT_MSG(tilemap->Origin.x >= 0 && tilemap->Origin.y >= 0, "Non 0, 0 world origin is current not supported");
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
	const Player* player = GetClientPlayer();

	Vector2i playerChunkCoord = TileToChunkCoord(tilemap, player->Transform.TilePos);

	// View distance checks + chunk loading
	if (player->HasMoved)
	{
		CheckChunksInLOS(tilemap);
	}
	
	// Update our chunks and handle removal
	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
		Vector2i difference = playerChunkCoord.Subtract(chunk->ChunkCoord);
		int chunkX = labs(difference.x);
		int chunkY = labs(difference.y);
		if (chunkX > VIEW_DISTANCE.x || chunkY > VIEW_DISTANCE.y)
		{
			tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
		}
		else
		{
			UpdateChunk(tilemap, chunk, game);
		}
	}

	while (tilemap->ChunksToUnload.HasNext())
	{
		Vector2i chunkCoord = tilemap->ChunksToUnload.PopValue();
		UnloadChunk(tilemap, chunkCoord);
	}

	// Draw tilemap
	Draw(tilemap, game);
}

void LateUpdate(ChunkedTileMap* tilemap, Game* game)
{
	if (!game->DebugTileView) return;
	Rectangle screen;
	screen.x = GetGameApp()->ScreenXY.x;
	screen.y = GetGameApp()->ScreenXY.y;
	screen.width = (float)GetScreenWidth();
	screen.height = (float)GetScreenHeight();
	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		CTileMap::TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
		if (CheckCollisionRecs(screen, chunk->Bounds))
		{
			DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
		}
	}
}

void UpdateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk, Game* game)
{
	GetGameApp()->NumOfChunksUpdated++;
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	if (IsChunkLoaded(tilemap, coord)) return nullptr;

	TileMapChunk* chunk = tilemap->ChunksList.PushNew();
	chunk->ChunkCoord = coord;

	const float chunkDimensionsPixel = (float)CHUNK_DIMENSIONS * TILE_SIZE_F;
	chunk->Bounds.x = (float)coord.x * chunkDimensionsPixel;
	chunk->Bounds.y = (float)coord.y * chunkDimensionsPixel;
	chunk->Bounds.width = chunkDimensionsPixel;
	chunk->Bounds.height = chunkDimensionsPixel;
	return chunk;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		auto chunkPtr = tilemap->ChunksList.PeekAt(i);
		if (chunkPtr->ChunkCoord.Equals(coord))
		{
			tilemap->ChunksList.RemoveAtFast(i);
			SLOG_INFO("[ Chunk ] Unloaded chunk (%s)", FMT_VEC2I(coord));
			break;
		}
	}
}

void 
GenerateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk)
{
	SASSERT(tilemap);
	SASSERT(chunk);
	if (chunk->IsChunkGenerated) return;
	chunk->IsChunkGenerated = true;

	for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
	{
		for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
		{
			uint32_t tileId = (uint32_t)SRandNextRange(GetGlobalRandom(), 1, 1);
			const auto& tile = CreateTileId(
				&GetGameApp()->Game->TileMgr,
				tileId);
			chunk->Tiles[x + y * CHUNK_DIMENSIONS] = tile;
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
		tilePos.x < tilemap->WorldDimTiles.x &&
		tilePos.y < tilemap->WorldDimTiles.y);
}

bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos)
{
	return (chunkPos.x >= tilemap->Origin.x &&
		chunkPos.y >= tilemap->Origin.y &&
		chunkPos.x < tilemap->WorldDimChunks.x &&
		chunkPos.y < tilemap->WorldDimChunks.y);
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
	ChunkCoord coord = TileToChunkCoord(tilemap, tileCoord);
	for (size_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		const auto& chunk = tilemap->ChunksList[i];
		if (chunk.ChunkCoord.Equals(coord))
			return &tilemap->ChunksList.Memory[i];
	}
	return nullptr;
}

ChunkCoord 
TileToChunkCoord(ChunkedTileMap* tilemap,TileCoord tilePos)
{
	ChunkCoord result;
	result.x = tilePos.x / CHUNK_DIMENSIONS;
	result.y = tilePos.y / CHUNK_DIMENSIONS;
	return result;
}

uint64_t 
TileToIndex(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	int tileChunkX = tilePos.x % CHUNK_DIMENSIONS;
	int tileChunkY = tilePos.y % CHUNK_DIMENSIONS;
	return static_cast<uint64_t>(tileChunkX + tileChunkY * CHUNK_DIMENSIONS);
}

void 
SetTile(ChunkedTileMap* tilemap, const Tile* tile, TileCoord tilePos)
{
	SASSERT(tilemap->ChunksList.IsAllocated());
	SASSERT(IsTileInBounds(tilemap, tilePos));
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk)
	{
		SLOG_WARN("SETTING tile[%d, %d] to "
			"unloaded or nonexistent chunk[%d, %d]",
			tilePos.x, tilePos.y, chunkCoord.x, chunkCoord.y);
		return;
	}
	SASSERT(chunk->IsChunkGenerated);
	uint64_t index = TileToIndex(tilemap, tilePos);
	chunk->Tiles[index] = *tile;
	//chunk->Tiles.Set(index, tile);
}

Tile* GetTile(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	SASSERT(tilemap->ChunksList.IsAllocated());
	SASSERT(IsTileInBounds(tilemap, tilePos));
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk)
	{
		SLOG_WARN("GETTING tile[%d, %d] to "
			"unloaded or nonexistent chunk[%d, %d]",
			tilePos.x, tilePos.y, chunkCoord.x, chunkCoord.y);
		return nullptr;
	}
	SASSERT(chunk->IsChunkGenerated);
	uint64_t index = TileToIndex(tilemap, tilePos);
	return &chunk->Tiles[index];
	//return chunk->Tiles.PeekAtPtr(index);
}

TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos)
{
	Vector2i v;
	v.x = (int)(pos.x) / TILE_SIZE;
	v.y = (int)(pos.y) / TILE_SIZE;
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
	SASSERT(tile);
	return tile->GetTileData(&GetGameApp()->Game->TileMgr)->Type == TileType::Solid;
}

internal void
CheckChunksInLOS(ChunkedTileMap* tilemap)
{
	Vector2i target = GetClientPlayer()->Transform.TilePos;
	Vector2i start = target.Subtract(tilemap->ViewDistance);
	Vector2i end = target.Add(tilemap->ViewDistance);
	for (int chunkY = start.y; chunkY <= end.y; ++chunkY)
	{
		for (int chunkX = start.x; chunkX <= end.x; ++chunkX)
		{
			ChunkCoord nextChunkCoord = { chunkX, chunkY };
			if (!IsChunkInBounds(tilemap, nextChunkCoord)) continue;
			TileMapChunk* chunk = LoadChunk(tilemap, nextChunkCoord);
			if (!chunk) continue;
			SLOG_INFO("[ Chunk ] Loaded chunk (%s). Was generated: %s", FMT_VEC2I(nextChunkCoord), FMT_BOOL(chunk->IsChunkGenerated));
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
			}
		}
	}
}

internal void
Draw(ChunkedTileMap* tilemap, Game* game)
{
	Texture2D* texture = &game->Resources.Atlas.Texture;
	TileMgr* tileMgr = &game->TileMgr;

	Vector2 cullTopLeft = Vector2Divide(GetGameApp()->ScreenXY, { TILE_SIZE_F, TILE_SIZE_F });
	for (int y = 0; y < SCREEN_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH_TILES; ++x)
		{
			TileCoord coord;
			coord.x = x + (int)cullTopLeft.x;
			coord.y = y + (int)cullTopLeft.y;
			if (!IsTileInBounds(tilemap, coord)) continue;

			TileMapChunk* chunk = GetChunkByTile(tilemap, coord);
			SASSERT(chunk);
			SASSERT(chunk->IsChunkGenerated);

			uint64_t index = TileToIndex(tilemap, coord);
			Tile* tile = &chunk->Tiles[index];
			Rectangle position
			{
				(float)(coord.x * TILE_SIZE),
				(float)(coord.y * TILE_SIZE),
				(float)TILE_SIZE,
				(float)TILE_SIZE
			};

			Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			if (tile->LOS == TileLOS::NoVision)
				color.w = 0.2f;
			tile->LOS = TileLOS::NoVision;

			LightMapSetCeiling(&game->LightMap, coord, tile->HasCeiling);

			ScalDrawTextureProF(
				texture,
				tile->GetTileTexData(tileMgr)->TexCoord,
				position,
				color);

			if (game->DebugTileView)
				DrawRectangleLinesEx(position, 1.0f, PINK);
		}
	}
}

}
