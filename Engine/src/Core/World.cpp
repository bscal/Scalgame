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
		{ 0, 0 }, { 16, 16 }, { 64, 64 });
	world->TileScale = { 16, 16 };

	world->EntityMgr.CreatePlayer(world);

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

void WorldUpdate(World* world, GameApplication* gameApp)
{
	ChunkedTileMap::Update(&world->ChunkedTileMap, gameApp);
	world->EntityMgr.Update(gameApp->Game, gameApp->DeltaTime);
}

bool IsInBounds(Vector2i startPos, Vector2i endPos,
	Vector2i current)
{
	return (current.x >= startPos.x &&
		current.y >= startPos.y &&
		current.x <= endPos.x &&
		current.y <= endPos.y);
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

Vector2 WorldPosToTilePos(World* world, Vector2 pos)
{
	Vector2 v;
	v.x = floorf(pos.x / world->TileScale.x);
	v.y = floorf(pos.y / world->TileScale.y);
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
