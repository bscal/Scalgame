#include "World.h"

#include "Game.h"
#include "ResourceManager.h"
#include "Creature.h"

#include <assert.h>

bool WorldInitialize(World* world, GameApplication* gameApp)
{
	assert(world);

	world->TileCoordsInLOS.max_load_factor(0.75f);
	world->TileCoordsInLOS.reserve(256);
	world->EntityActionsList.Initialize();
	CTileMap::Initialize(&world->ChunkedTileMap,
		{ 16, 16 }, { 0, 0 }, { 2, 2 }, { 32, 32 });

	world->EntityMgr.CreatePlayer(world);

	world->LightMap.Initialize(1600 / 16, 900 / 16);
	world->SightMap.Initialize(112, 80);

	TileMgrInitialize(&world->TileMgr,
		&gameApp->Game->Atlas);

	world->IsInitialized = true;
	return true;
}

void WorldLoad(World* world, Game* game)
{
	CTileMap::Update(&world->ChunkedTileMap, game);

	world->IsLoaded = true;
}

void WorldFree(World* world)
{
	if (!world)
	{
		S_LOG_ERR("Trying to unload a null world");
		return;
	}
	world->IsLoaded = false;
	world->IsInitialized = false;
	CTileMap::Free(&world->ChunkedTileMap);
	world->EntityActionsList.Free();
}

void WorldUpdate(World* world, Game* game)
{
	const auto& playerTilePos = GetClientPlayer()->Transform.TilePos;
	world->LightMap.UpdatePositions(playerTilePos);
	world->LightMap.Update(world);

	GetGameApp()->NumOfChunksUpdated = 0;
	CTileMap::Update(&world->ChunkedTileMap, game);

	const double drawStart = GetTime();
	world->SightMap.Update(world, playerTilePos);
	GetGameApp()->LOSTime = GetTime() - drawStart;
	GetGameApp()->NumOfLoadedChunks = 
		world->ChunkedTileMap.ChunksList.Length;

	world->EntityMgr.Update(game, GetDeltaTime());

	CTileMap::LateUpdateChunk(&world->ChunkedTileMap, game);
	world->LightMap.LateUpdate(world);
}

void LateWorldUpdate(World* world, Game* game)
{
	Rectangle rect;
	rect.x = world->LightMap.StartPos.x * 16.0f;
	rect.y = world->LightMap.StartPos.y * 16.0f;
	rect.width = world->LightMap.Width * 16.0f;
	rect.height = world->LightMap.Height * 16.0f;
	DrawRectangleLinesEx(rect, 4, ORANGE);
}

bool CanMoveToTile(World* world, Vector2i position)
{
	if (!WorldIsInBounds(world, position))
	{
		return false;
	}
	const auto tile = CTileMap::GetTile(&world->ChunkedTileMap,
		position);
	return true;
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
