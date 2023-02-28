#include "ChunkedTileMap.h"

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

// TODO
static SRandom Random;

void Initialize(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap);
	SASSERT(!tilemap->ChunksList.IsAllocated());

	SRandomInitialize(&Random, 0);

	Vector2i origin = { 0, 0 };
	Vector2i worldDim = { 4, 4 };

	tilemap->TileSize = { TILE_SIZE, TILE_SIZE };
	tilemap->ChunkSize = { CHUNK_DIMENSIONS, CHUNK_DIMENSIONS };
	tilemap->Origin = origin;
	tilemap->WorldDimChunks = worldDim;

	tilemap->WorldDimTiles.x = tilemap->WorldDimChunks.x
		* tilemap->ChunkSize.x;
	tilemap->WorldDimTiles.y = tilemap->WorldDimChunks.y
		* tilemap->ChunkSize.y;

	tilemap->ViewDistance.x = VIEW_DISTANCE.x;
	tilemap->ViewDistance.y = VIEW_DISTANCE.y;

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

	uint32_t capacity = (uint32_t)labs(tilemap->ViewDistance.x * tilemap->ViewDistance.y);
	tilemap->ChunksList.Resize(capacity);

	SASSERT_MSG(tilemap->Origin.x >= 0 && tilemap->Origin.y >= 0, "Non 0, 0 world origin is current not supported");
}

void Free(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap->ChunksList.IsAllocated());
	tilemap->ChunksList.Free();
	tilemap->ChunksToUnload.Free();
}

void 
FindChunksInView(ChunkedTileMap* tilemap, Game* game, Vector2i centerChunk)
{
	int startX = centerChunk.x - (int)tilemap->ViewDistance.x;
	int startY = centerChunk.y - (int)tilemap->ViewDistance.y;
	int endX = centerChunk.x + (int)tilemap->ViewDistance.x;
	int endY = centerChunk.y + (int)tilemap->ViewDistance.y;
	for (int chunkY = startY; chunkY <= endY; ++chunkY)
	{
		for (int chunkX = startX; chunkX <= endX; ++chunkX)
		{
			ChunkCoord nextChunkCoord = { chunkX, chunkY };
			if (!IsChunkInBounds(tilemap, nextChunkCoord)) continue;
			if (IsChunkLoaded(tilemap, nextChunkCoord)) continue;
			TileMapChunk* chunk = LoadChunk(tilemap, nextChunkCoord);
			SLOG_INFO("ChunkLoaded");
			if (!chunk->IsChunkGenerated)
			{
				GenerateChunk(tilemap, chunk);
			}
		}
	}
}

internal void 
UpdateChunks(ChunkedTileMap* tilemap, Game* game, ChunkCoord playerChunkCoord)
{
	for (uint16_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
		Vector2i difference = playerChunkCoord.Subtract(chunk->ChunkCoord);
		int chunkX = labs(difference.x);
		int chunkY = labs(difference.y);
		if (chunkX > VIEW_DISTANCE.x || chunkY > VIEW_DISTANCE.y)
		{
			tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
			SLOG_INFO("[ Chunk ] Chunk marked for removal (x: %d, y: %d)",
				chunk->ChunkCoord.x, chunk->ChunkCoord.y);
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
}

internal void 
Draw(ChunkedTileMap* tilemap, Game* game)
{
	const Texture2D* texture = &game->Resources.Atlas.Texture;
	TileMgr* tileMgr = &game->TileMgr;

	Vector2i target = GetClientPlayer()->Transform.TilePos;
	const float lPadding = 2.0f;
	float offsetX = game->WorldCamera.offset.x / TILE_SIZE_F + lPadding;
	float offsetY = game->WorldCamera.offset.y / TILE_SIZE_F + lPadding;
	int tileOffsetX = (target.x - (int)offsetX);
	int tileOffsetY = (target.y - (int)offsetY);
	for (int y = 0; y < SCREEN_HEIGHT_TILES; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH_TILES; ++x)
		{
			TileCoord coord = {
				x + tileOffsetX,
				y + tileOffsetY
			};
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

			//game->DebugDisableFOV

			Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			if (tile->LOS == TileLOS::NoVision)
				color.w -= 0.4f;
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

void Update(ChunkedTileMap* tilemap, Game* game)
{
	const Player* player = GetClientPlayer();
	Vector2i playerChunkCoord = TileToChunkCoord(tilemap, player->Transform.TilePos);
	if (player->HasMoved)
	{
		FindChunksInView(tilemap, game, playerChunkCoord);
	}

	Draw(tilemap, game);
	UpdateChunks(tilemap, game, playerChunkCoord);
}

void LateUpdateChunk(ChunkedTileMap* tilemap, Game* game)
{
	if (!game->DebugTileView) return; 
	const auto& screenRect = GetScaledScreenRect();

	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		CTileMap::TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
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
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap,
	ChunkCoord coord)
{
	TileMapChunk* chunk = tilemap->ChunksList.PushNew();
	chunk->Entities.Allocator = SMEM_GAME_ALLOCATOR;
	chunk->Entities.Resize(10); // TODO
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
	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		auto chunkPtr = tilemap->ChunksList.PeekAt(i);
		if (chunkPtr->ChunkCoord.Equals(coord))
		{
			chunkPtr->Entities.Free();
			tilemap->ChunksList.RemoveAtFast(i);
			SLOG_INFO("ChunkUnloaded");
			break;
		}
	}
}

void GenerateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk)
{
	SASSERT(chunk);
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
		tilePos.x < tilemap->WorldDimTiles.x&&
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

void SetTile(ChunkedTileMap* tilemap,
	const Tile* tile, TileCoord tilePos)
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

const Tile& GetTileRef(ChunkedTileMap* tilemap,
	TileCoord tilePos)
{
	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	SASSERT(chunk);
	SASSERT(chunk->IsChunkGenerated);
	// TODO should be always SASSERT if chunk doesnt exist/not generated
	uint64_t index = TileToIndex(tilemap, tilePos);
	SASSERT(index < tilemap->ChunkTileCount);
	return chunk->Tiles[index];
}

TileCoord WorldToTile(ChunkedTileMap* tilemap, Vector2 pos)
{
	Vector2i v;
	v.x = (int)(pos.x) / TILE_SIZE;
	v.y = (int)(pos.y) / TILE_SIZE;
	return v;
}

void SetColor(ChunkedTileMap* tilemap, TileCoord tileCoord, Vector4 color)
{
	if (!IsTileInBounds(tilemap, tileCoord)) return;
	TileMapChunk* chunk = GetChunkByTile(tilemap, tileCoord);
	uint32_t index = TileToIndex(tilemap, tileCoord);
	//chunk->TileColors[index] = color;
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

}
