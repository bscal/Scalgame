#include "World.h"

#include "Game.h"
#include "ResourceManager.h"
#include "EntityMgr.h"
#include "Creature.h"
#include "Sprite.h"
#include "Lighting.h"

void WorldInitialize(World* world, GameApplication* gameApp)
{
	CTileMap::Initialize(&world->ChunkedTileMap);

	world->IsAllocated = true;
	SLOG_INFO("[ WORLD ] Successfully initialized world!");
}

void WorldLoad(World* world, Game* game)
{
	LightsInitialized(GetGameApp());

	// FIXME: find better location for this
	MapGenInitialize(&game->MapGen, 0);

	CTileMap::Load(&world->ChunkedTileMap);

	world->IsLoaded = true;
	SLOG_INFO("[ WORLD ] World loaded!");
}

void WorldFree(World* world)
{
	SASSERT(world);
	if (!world->IsAllocated || !world->IsLoaded)
	{
		SLOG_ERR("Trying to unload a null world");
		return;
	}
	world->IsLoaded = false;
	world->IsAllocated = false;
	CTileMap::Free(&world->ChunkedTileMap);
	world->EntityActionsList.Free();
}

void WorldUpdate(World* world, Game* game)
{
	PROFILE_BEGIN();
	GetGameApp()->NumOfChunksUpdated = 0;

	LightsUpdate(game);

	//CTileMap::Update(&world->ChunkedTileMap, game);

	GetGameApp()->NumOfLoadedChunks = (int)world->ChunkedTileMap.ChunksList.Count;
	PROFILE_END();
}

void WorldLateUpdate(World* world, Game* game)
{
	CTileMap::LateUpdate(&world->ChunkedTileMap, game);
}

bool CanMoveToTile(World* world, Vector2i position)
{
	if (!WorldIsInBounds(world, position))
		return false;

	TileData* tile = CTileMap::GetTile(&world->ChunkedTileMap, position);
	return tile->GetTile()->Type == TileType::Floor;
}

bool WorldIsInBounds(World* world, Vector2i pos)
{
	return CTileMap::IsTileInBounds(&world->ChunkedTileMap, pos);
}

void TurnEnd(World* world, Game* game, int timeChange)
{
	
}

void AddAction(World* world, Action* action)
{
	auto length = world->EntityActionsList.Count;
	if (length < 1)
	{
		world->EntityActionsList.Push(action);
		return;
	}

	bool added = false;
	for (int i = 0; i < length; ++i)
	{
		Action* at = world->EntityActionsList.PeekAt(i);
		if (at->Cost > action->Cost)
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
	for (int i = 0; i < world->EntityActionsList.Count; ++i)
	{
		Action* at = world->EntityActionsList.PeekAt(i);
		at->ActionFunction(world, at);
	}
}

Vector2i WorldToTileCoord(World* world, Vector2 tile)
{
	return CTileMap::WorldToTile(&world->ChunkedTileMap, tile);
}
