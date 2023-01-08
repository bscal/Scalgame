#include "World.h"

#include "Game.h"
#include "ResourceManager.h"
#include "EntityMgr.h"
#include "Creature.h"
#include "Sprite.h"
#include "Lighting.h"

#include <assert.h>

void WorldInitialize(World* world, GameApplication* gameApp)
{
	world->TileCoordsInLOS.max_load_factor(0.75f);
	world->TileCoordsInLOS.reserve(256);
	world->EntityActionsList.Initialize();

	CTileMap::Initialize(&world->ChunkedTileMap, gameApp->Game);

	world->IsInitialized = true;
	SLOG_INFO("[ WORLD ] Successfully initialized world!");
}

void WorldLoad(World* world, Game* game)
{
	LightsInitialized(GetGameApp());

	// Load chunks around players at start
	CTileMap::FindChunksInView(&world->ChunkedTileMap, game);

	world->IsLoaded = true;
	SLOG_INFO("[ WORLD ] World loaded!");
}

void WorldFree(World* world)
{
	assert(world);
	if (!world->IsInitialized || !world->IsLoaded)
	{
		SLOG_ERR("Trying to unload a null world");
		return;
	}
	world->IsLoaded = false;
	world->IsInitialized = false;
	CTileMap::Free(&world->ChunkedTileMap);
	world->EntityActionsList.Free();
}

void WorldUpdate(World* world, Game* game)
{
	GetGameApp()->NumOfChunksUpdated = 0;

	LightsUpdate(game);

	CTileMap::Update(&world->ChunkedTileMap, game);
	CTileMap::LateUpdateChunk(&world->ChunkedTileMap, game);

	GetGameApp()->NumOfLoadedChunks = (int)world->ChunkedTileMap.ChunksList.Count;
}

bool CanMoveToTile(World* world, Vector2i position)
{
	if (!WorldIsInBounds(world, position))
	{
		return false;
	}
	const auto tile = CTileMap::GetTile(&world->ChunkedTileMap,
		position);
	const auto tileData = tile->GetTileData(&GetGame()->TileMgr);
	return tileData.Type == TileType::Floor;
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
	for (int i = 0; i < world->EntityActionsList.Count; ++i)
	{
		Action at = world->EntityActionsList.PeekAt(i);
		at.ActionFunction(world, &at);
	}
}

Vector2i WorldTileScale(World* world)
{
	return world->ChunkedTileMap.TileSize;
}

Vector2i WorldToTileCoord(World* world, Vector2 tile)
{
	return CTileMap::WorldToTile(&world->ChunkedTileMap, tile);
}
