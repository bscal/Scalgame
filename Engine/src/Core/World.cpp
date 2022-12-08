#include "World.h"

#include "Game.h"
#include "ResourceManager.h"
#include "Creature.h"

#include <assert.h>

bool WorldInitialize(World* world, TileSet* tileset)
{
	assert(world);
	assert(tileset);

	world->TileCoordsInLOS.max_load_factor(0.75f);
	world->TileCoordsInLOS.reserve(256);
	world->EntityActionsList.Initialize();
	ChunkedTileMap::Initialize(&world->ChunkedTileMap, tileset,
		{ 16, 16 }, { 0, 0 }, { 8, 8 }, { 32, 32 });
	world->EntityMgr.CreatePlayer(world);
	world->LightMap.Initialize(112, 80);
	world->SightMap.Initialize(112, 80);

	world->IsLoaded = true;
	return true;
}

void WorldFree(World* world)
{
	if (!world)
	{
		S_LOG_ERR("Trying to unload a null world");
		return;
	}
	world->IsLoaded = false;
	ChunkedTileMap::Free(&world->ChunkedTileMap);
	world->EntityActionsList.Free();
}

void WorldUpdate(World* world, Game* game)
{
	const auto& playerTilePos = GetClientPlayer()->Transform.TilePos;
	world->LightMap.UpdatePositions(playerTilePos);

	GetGameApp()->NumOfChunksUpdated = 0;
	ChunkedTileMap::Update(&world->ChunkedTileMap, game);

	const double drawStart = GetTime();
	world->SightMap.Update(world, playerTilePos);
	GetGameApp()->LOSTime = GetTime() - drawStart;
	GetGameApp()->NumOfLoadedChunks = 
		world->ChunkedTileMap.ChunksList.Length;

	world->EntityMgr.Update(game, GetDeltaTime());

	ChunkedTileMap::LateUpdateChunk(&world->ChunkedTileMap, game);
}

void LateWorldUpdate(World* world, Game* game)
{
	Rectangle rect;
	rect.x = world->LightMap.StartPos.x * 16.0f;
	rect.y = world->LightMap.StartPos.y * 16.0f;
	rect.width = world->LightMap.Width * 16.0f;
	rect.height = world->LightMap.Height * 16.0f;
	DrawRectangleLinesEx(rect, 4, PURPLE);
}

bool CanMoveToTile(World* world, Vector2i position)
{
	if (!IsInBounds(world->ChunkedTileMap.BoundsStart,
		world->ChunkedTileMap.BoundsEnd, position))
	{
		return false;
	}
	const auto tile = ChunkedTileMap::GetTile(&world->ChunkedTileMap,
		position.x, position.y);
	return true;
}

bool IsInWorldBounds(World* world, Vector2i pos)
{
	return IsInBounds(world->ChunkedTileMap.BoundsStart,
		world->ChunkedTileMap.BoundsEnd, pos);
}

Vector2 WorldPosToTilePos(World* world, Vector2 pos)
{
	Vector2 v;
	v.x = floorf(pos.x / world->ChunkedTileMap.TileScale.x);
	v.y = floorf(pos.y / world->ChunkedTileMap.TileScale.y);
	return v;
}

void TurnEnd(World* world, Game* game, int timeChange)
{
	
}

void AddAction(World* world, Action* action)
{
	auto length = world->EntityActionsList.Length;
	if (length < 1)
	{
		world->EntityActionsList.Push(action);
		return;
	}

	bool added = false;
	for (int i = 0; i < length; ++i)
	{
		Action at = world->EntityActionsList.PeekAt(i);
		if (at.Cost > action->Cost)
		{
			world->EntityActionsList.PushAt(i, action);
			added = true;
			break;
		}
	}

	if (!added)
	{
		world->EntityActionsList.Push(action);
	}
}

void ProcessActions(World* world)
{
	for (int i = 0; i < world->EntityActionsList.Length; ++i)
	{
		Action at = world->EntityActionsList.PeekAt(i);
		at.ActionFunction(world, &at);
	}
}
