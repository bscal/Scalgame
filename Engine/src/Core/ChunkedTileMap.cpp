#include "ChunkedTileMap.h"

#include "Game.h"
#include "SRandom.h"
#include "SUtil.h"
#include "Structures/SLinkedList.h"

#include <assert.h>

namespace ChunkedTileMap
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

void Initialize(ChunkedTileMap* tilemap, TileSet* tileSet, Vector2i mapStartPos,
	Vector2i mapEndPos, Vector2i chunkDimensions)
{
	if (!tilemap)
	{
		S_LOG_ERR("ChunkedTileMap: tilemap cannot be nullptr");
		return;
	}
	if (!tileSet)
	{
		S_LOG_ERR("ChunkedTileMap: tileSet cannot be nullptr");
		return;
	}
	if (mapStartPos.x > mapEndPos.x)
	{
		S_LOG_ERR("ChunkedTileMap: mapStartPos.x cannot be > mapEndPos.x");
		return;
	}
	if (mapStartPos.y > mapEndPos.y)
	{
		S_LOG_ERR("ChunkedTileMap: mapStartPos.y cannot be > mapEndPos.y");
		return;
	}
	if (chunkDimensions.x <= 0 || chunkDimensions.y <= 0)
	{
		S_LOG_ERR("ChunkedTileMap: chunkDimensions must be > than 0");
		return;
	}

	SRandomInitialize(&Random, 0);

	tilemap->TileSet = tileSet;
	tilemap->StartChunkCoords = mapStartPos;
	tilemap->EndChunkCoords = mapEndPos;
	tilemap->ChunkDimensions = chunkDimensions;
	tilemap->ChunkTileSize = (size_t)chunkDimensions.x * (size_t)chunkDimensions.y;
	tilemap->BoundsStart.x = mapStartPos.x * 16;
	tilemap->BoundsStart.y = mapStartPos.y * 16;
	tilemap->BoundsEnd.x = mapEndPos.x * 16;
	tilemap->BoundsEnd.y = mapEndPos.y * 16;
	tilemap->ViewDistance.x = 3.0f;
	tilemap->ViewDistance.y = 2.0f;
	size_t capacity = (size_t)(tilemap->ViewDistance.x * tilemap->ViewDistance.y);
	tilemap->ChunksList.InitializeCap(capacity);
	size_t mapCapacity = (size_t)((float)capacity + (float)capacity * .5f);
	tilemap->ChunksMap.InitializeEx(mapCapacity, &ChunkMapHash, &ChunkMapEquals);
}

void Free(ChunkedTileMap* tilemap)
{
	if (!tilemap)
	{
		S_LOG_ERR("ThreadedTileMapFree: tilemap cannot be nullptr");
		return;
	}
	assert(tilemap->ChunksList.IsInitialized());
	assert(tilemap->ChunksMap.IsInitialized());
	tilemap->ChunksList.Free();
	tilemap->ChunksMap.Free();
	SLinkedFree(&tilemap->ChunksToUnload);
}

void Create(ChunkedTileMap* tilemap, int loadWidth,
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

void FindChunksInView(ChunkedTileMap* tilemap, Game* game)
{
	auto& player = game->World.EntityMgr.Players[0];

	ChunkCoord chunkCoord = GetWorldTileToChunkCoord(tilemap,
		player.Transform.TilePos.x, (int)player.Transform.TilePos.y);

	int startX = chunkCoord.x - (int)tilemap->ViewDistance.x;
	int endX = chunkCoord.x + (int)tilemap->ViewDistance.x;
	int startY = chunkCoord.y - (int)tilemap->ViewDistance.y;
	int endY = chunkCoord.y + (int)tilemap->ViewDistance.y;
	for (int chunkY = startY; chunkY < endY; ++chunkY)
	{
		for (int chunkX = startX; chunkX < endX; ++chunkX)
		{
			ChunkCoord nextChunkCoord = { chunkX, chunkY };
			if (!nextChunkCoord.IsInBounds(
				tilemap->StartChunkCoords, tilemap->EndChunkCoords)) continue;
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

void Update(ChunkedTileMap* tilemap, GameApplication* gameApp)
{
	assert(tilemap);
	Camera2D camera = gameApp->Game->Camera;
	Rectangle rect = 
	{
		camera.target.x, camera.target.y,
		(float)GetScreenWidth() / camera.zoom,
		(float)GetScreenHeight() / camera.zoom
	};
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(rect, chunk->Bounds))
		{
			UpdateChunk(tilemap, chunk, gameApp);
		}
	}
	FindChunksInView(tilemap, gameApp->Game);
	//DrawRectangleLinesEx(rect, 4, ORANGE);
}

void LateUpdateChunk(ChunkedTileMap* tilemap,GameApplication* gameApp)
{
	Camera2D camera = gameApp->Game->Camera;
	Rectangle rect =
	{
		camera.target.x, camera.target.y,
		(float)GetScreenWidth() / camera.zoom,
		(float)GetScreenHeight() / camera.zoom
	};
	for (uint64_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		auto chunk = tilemap->ChunksList.PeekAtPtr(i);
		if (CheckCollisionRecs(rect, chunk->Bounds))
		{
			DrawRectangleLinesEx(chunk->Bounds, 4, GREEN);
		}
	}

}

void UpdateChunk(ChunkedTileMap* tilemap,
	TileMapChunk* chunk, GameApplication* gameApp)
{
	//DrawRectangleLinesEx(chunk->Bounds, 2, GREEN);
	assert(chunk);
	assert(chunk->IsChunkGenerated);
	for (int y = 0; y < tilemap->ChunkDimensions.y; ++y)
	{
		for (int x = 0; x < tilemap->ChunkDimensions.x; ++x)
		{
			uint64_t index = x + y * tilemap->ChunkDimensions.x;
			TileMapTile tile = chunk->Tiles[index];

			//Rectangle textureRect;
			//textureRect.x = (float)tile.TextureCoord.x;
			//textureRect.y = (float)tile.TextureCoord.y;
			//textureRect.width = (float)tile.TextureCoord.w;
			//textureRect.height = (float)tile.TextureCoord.h;

			float worldX = (float)chunk->ChunkCoord.x *
				((float)tilemap->ChunkDimensions.x * 16.0f);
			float worldY = (float)chunk->ChunkCoord.y *
				((float)tilemap->ChunkDimensions.y * 16.0f);
			Vector3 worldPosition;
			worldPosition.x = worldX + (float)x * 16.0f;
			worldPosition.y = worldY + (float)y * 16.0f;
			worldPosition.z = 0;

			//DrawBillboardPro(
			//	gameApp->Game->Camera3D,
			//	gameApp->Game->Atlas.Texture,
			//	tile.TextureCoord,
			//	worldPosition,
			//	{ 0.0f, 1.0f, 0.0f },
			//	{ 16.0f, 16.0f },
			//	{ 0.0f, 0.0f },
			//	0.0f,
			//	WHITE
			//);

			DrawTextureRec(
				gameApp->Game->Atlas.Texture,
				tile.TextureCoord,
				{
								worldPosition.x, worldPosition.y
				},
				WHITE);
		}
	}
}

TileMapChunk* LoadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk chunk = {};
	chunk.Tiles.InitializeCap(tilemap->ChunkTileSize);
	chunk.ChunkCoord = coord;
	chunk.Bounds.x = (float)coord.x * 64.0f * 16.0f;
	chunk.Bounds.y = (float)coord.y * 64.0f * 16.0f;
	chunk.Bounds.width = (float)tilemap->ChunkDimensions.x * 16.0f;
	chunk.Bounds.height = (float)tilemap->ChunkDimensions.y * 16.0f;

	tilemap->ChunksList.Push(&chunk);
	TileMapChunk* chunkPtrInChunksList = tilemap->ChunksList.Last();
	tilemap->ChunksMap.Put(&coord, &chunkPtrInChunksList);
	return chunkPtrInChunksList;
}

void UnloadChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&coord);
	if (!chunk) return;

	tilemap->ChunksMap.Remove(&coord);

	// TODO maybe look into storing index in Chunk, and swapping
	// with last chunk here?
	for (size_t i = 0; i < tilemap->ChunksList.Length; ++i)
	{
		if (tilemap->ChunksList[i].ChunkCoord.Equals(coord))
		{
			tilemap->ChunksList.RemoveAtFast(i);
		}
	}
}

void GenerateChunk(ChunkedTileMap* tilemap, TileMapChunk* chunk)
{
	assert(chunk);
	if (chunk->IsChunkGenerated)
	{
		S_LOG_WARN("Trying to generate an already generated chunk!");
		return;
	}
	chunk->IsChunkGenerated = true;

	for (int y = 0; y < tilemap->ChunkDimensions.y; ++y)
	{
		for (int x = 0; x < tilemap->ChunkDimensions.x; ++x)
		{
			uint32_t tileId = (uint32_t)SRandNextRange(&Random, 3, 4);
			TileMapTile tile = {};
			tile.TileId = tileId;

			uint32_t rect;
			if (tileId == 4)
				rect = GetGameApp()->Game->Atlas.SpritesByName["Tile2"];
			else
				rect = GetGameApp()->Game->Atlas.SpritesByName["Tile3"];
			//auto rect = GetGameApp()->Game->Atlas.SpritesArray[tileId];
			tile.TextureCoord = GetGameApp()->Game->Atlas.SpritesArray[rect];
			//uint16_t texX = (tileId % 16) * 16;
			//uint16_t texY = (tileId / 16) * 16;
			//tile.TextureCoord.x = texX;
			//tile.TextureCoord.y = texY;
			//tile.TextureCoord.width = 16;
			//tile.TextureCoord.height = 16;
			tile.FowLevel = 0;
			chunk->Tiles.Push(&tile);
		}
	}
}

bool IsChunkLoaded(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	return tilemap->ChunksMap.Contains(&coord);
}

TileMapChunk* GetChunk(ChunkedTileMap* tilemap, ChunkCoord coord)
{
	return *tilemap->ChunksMap.Get(&coord);
}

ChunkCoord GetWorldTileToChunkCoord(ChunkedTileMap* tilemap,
	int tileX, int tileY)
{
	ChunkCoord result;
	result.x = tileX / 16 / tilemap->ChunkDimensions.x;
	result.y = tileY / 16 / tilemap->ChunkDimensions.y;
	return result;
}

uint64_t GetWorldTileToChunkIndex(ChunkedTileMap* tilemap,
	int tileX, int tileY)
{
	int tileChunkX = tileX % tilemap->ChunkDimensions.x;
	int tileChunkY = tileY % tilemap->ChunkDimensions.y;
	return static_cast<uint64_t>(tileChunkX + tileChunkY * tilemap->ChunkDimensions.x);
}

void SetTile(ChunkedTileMap* tilemap, TileMapTile* tile,
	int tileX, int tileY)
{
	ChunkCoord chunkCoord = GetWorldTileToChunkCoord(tilemap, tileX, tileY);
	uint32_t tileChunkCoord = (uint32_t)GetWorldTileToChunkIndex(tilemap, tileX, tileY);
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&chunkCoord);
	if (!chunk)
	{
		#if SCAL_DEBUG
		assert(false);
		#endif
		return;
	}
	assert(((*chunk)->Tiles.IsInitialized()));
	(*chunk)->Tiles.Set(tileChunkCoord, tile);
}

TileMapTile* GetTile(ChunkedTileMap* tilemap,
	int tileX, int tileY)
{
	ChunkCoord chunkCoord = GetWorldTileToChunkCoord(tilemap, tileX, tileY);
	uint32_t tileChunkCoord = (uint32_t)GetWorldTileToChunkIndex(tilemap, tileX, tileY);
	TileMapChunk** chunk = tilemap->ChunksMap.Get(&chunkCoord);
	if (!chunk)
	{
		#if SCAL_DEBUG
		assert(false);
		#endif
		return nullptr;
	}
	assert(((*chunk)->Tiles.IsInitialized()));
	return (*chunk)->Tiles.PeekAtPtr(tileChunkCoord);
}

}
