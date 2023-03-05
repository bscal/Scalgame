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
internal void DrawChunks(ChunkedTileMap* tilemap);
internal void BakeChunk(TileMapChunk* chunk, Game* game);
internal void Draw(ChunkedTileMap* tilemap, Game* game);

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

	uint32_t capacity = (3 + tilemap->ViewDistance.x) * (3 + tilemap->ViewDistance.y) + 3;
	SASSERT(capacity > 0);
	tilemap->ChunksList.Reserve(capacity);
}

void Free(ChunkedTileMap* tilemap)
{
	SASSERT(tilemap->ChunksList.IsAllocated());

	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		// Function checks if texture is loaded
		UnloadRenderTexture(tilemap->ChunksList[i].Texture);
	}

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
		if (chunkX > VIEW_DISTANCE.x + 1 || chunkY > VIEW_DISTANCE.y + 1)
		{
			tilemap->ChunksToUnload.Push(&chunk->ChunkCoord);
		}
		else
		{
			GetGameApp()->NumOfChunksUpdated++;
			UpdateChunk(tilemap, chunk, game);
		}
	}

	while (tilemap->ChunksToUnload.HasNext())
	{
		Vector2i chunkCoord = tilemap->ChunksToUnload.PopValue();
		UnloadChunk(tilemap, chunkCoord);
	}

	DrawChunks(tilemap);

	// Draw tilemap
	// Draw(tilemap, game);
	PROFILE_END();
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
		TileMapChunk* chunk = tilemap->ChunksList.PeekAt(i);
		if (CheckCollisionRecs(screen, chunk->Bounds))
		{
			DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
		}
	}
}

void 
UpdateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk, Game* game)
{
	if (chunk->BakeState != ChunkBake::NoBuild)
	{
		BakeChunk(chunk, game);
		if (chunk->BakeState == ChunkBake::BuildNeighbors)
		{
			for (int i = 0; i < ARRAY_LENGTH(Vec2i_NEIGHTBORS_CORNERS); ++i)
			{
				Vector2i coord = chunk->ChunkCoord.Add(Vec2i_NEIGHTBORS_CORNERS[i]);
				if (!IsChunkInBounds(tilemap, coord)) continue;
				GetChunk(tilemap, coord)->BakeState = ChunkBake::BuildSelf;
			}
		}
	}
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	if (IsChunkLoaded(tilemap, coord)) return nullptr;

	TileMapChunk* chunk = tilemap->ChunksList.PushNew();
	SASSERT(chunk);

	chunk->ChunkCoord = coord;
	
	float size = (float)CHUNK_DIMENSIONS * TILE_SIZE_F;
	chunk->Texture = SLoadRenderTextureEx(size, size, PIXELFORMAT_UNCOMPRESSED_R8G8B8, false);

	const float chunkDimensionsPixel = (float)CHUNK_DIMENSIONS * TILE_SIZE_F;
	chunk->Bounds.x = (float)coord.x * chunkDimensionsPixel;
	chunk->Bounds.y = (float)coord.y * chunkDimensionsPixel;
	chunk->Bounds.width = chunkDimensionsPixel;
	chunk->Bounds.height = chunkDimensionsPixel;

	MapGenGenerateChunk(&GetGame()->MapGen, tilemap, chunk);

	BakeChunk(chunk, GetGame());

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
			UnloadRenderTexture(tilemap->ChunksList[tilemap->ChunksList.Count].Texture);
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
	return (tilePos.x >= 0 &&
		tilePos.y >= 0 &&
		tilePos.x < tilemap->WorldDimTiles.x &&
		tilePos.y < tilemap->WorldDimTiles.y);
}

bool IsChunkInBounds(ChunkedTileMap* tilemap, ChunkCoord chunkPos)
{
	return (chunkPos.x >= 0 &&
		chunkPos.y >= 0 &&
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
	SASSERT(tilemap);
	SASSERT(tile);
	SASSERT(tilemap->ChunksList.IsAllocated());
	SASSERT(IsTileInBounds(tilemap, tilePos));

	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
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
	uint64_t index = TileToIndex(tilemap, tilePos);
	chunk->Tiles[index] = *tile;
	chunk->BakeState = ChunkBake::BuildSelf;
}

Tile* 
GetTile(ChunkedTileMap* tilemap, TileCoord tilePos)
{
	SASSERT(tilemap);
	SASSERT(tilemap->ChunksList.IsAllocated());
	SASSERT(IsTileInBounds(tilemap, tilePos));

	ChunkCoord chunkCoord = TileToChunkCoord(tilemap, tilePos);
	TileMapChunk* chunk = GetChunk(tilemap, chunkCoord);
	if (!chunk || chunk->State == ChunkState::Unloaded)
	{
		SLOG_WARN("[ Tilemap ] GET failed! tile(%s), nonexistent chunk(%s)",
			FMT_VEC2I(tilePos), FMT_VEC2I(chunkCoord));
		return nullptr;
	}
	uint64_t index = TileToIndex(tilemap, tilePos);
	return &chunk->Tiles[index];
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

internal void DrawChunks(ChunkedTileMap* tilemap)
{
	PROFILE_BEGIN();
	for (uint32_t i = 0; i < tilemap->ChunksList.Count; ++i)
	{
		if (CheckCollisionRecs(tilemap->ChunksList[i].Bounds, GetGame()->CullingRect))
		{
			Rectangle src = tilemap->ChunksList[i].Bounds;
			src.height = -src.height;

			Rectangle dest = tilemap->ChunksList[i].Bounds;
			
			ScalDrawTextureProF(
				&tilemap->ChunksList[i].Texture.texture,
				src,
				dest,
				{ 1.f, 1.f, 1.f, 1.f }
			);
		}
	}
	PROFILE_END();
}

internal void
BakeChunk(TileMapChunk* chunk, Game* game)
{
	PROFILE_BEGIN();
	BeginTextureMode(chunk->Texture);
	Texture2D* texture = &game->Resources.Atlas.Texture;
	TileMgr* tileMgr = &game->TileMgr;
	for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
	{
		for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
		{
			int index = x + y * CHUNK_DIMENSIONS;
			Rectangle position
			{
				(float)x * TILE_SIZE_F,
				(float)y * TILE_SIZE_F,
				TILE_SIZE_F,
				TILE_SIZE_F
			};
			ScalDrawTextureProF(
				texture,
				tileMgr->TileTextureData[chunk->Tiles[index].TileId].TexCoord,
				position,
				{ 1.0f, 1.0f, 1.0f, 1.0f });
		}
	}
	chunk->BakeState = ChunkBake::NoBuild;
	EndTextureMode();
	PROFILE_END();
}

internal void
Draw(ChunkedTileMap* tilemap, Game* game)
{
	PROFILE_BEGIN();
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
			SASSERT(chunk->State != ChunkState::Unloaded);

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
	PROFILE_END();
}

}
