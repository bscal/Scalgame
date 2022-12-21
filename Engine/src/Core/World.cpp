#include "World.h"

#include "Game.h"
#include "ResourceManager.h"
#include "Creature.h"

#include <assert.h>

void WorldInitialize(World* world, GameApplication* gameApp)
{
	// TODO move
	TileMgrInitialize(&world->TileMgr,
		&gameApp->Game->Atlas);

	world->TileCoordsInLOS.max_load_factor(0.75f);
	world->TileCoordsInLOS.reserve(256);
	world->EntityActionsList.Initialize();
	world->EntityMgr.CreatePlayer(world);
	world->SightMap.Initialize(112, 80);

	CTileMap::Initialize(&world->ChunkedTileMap, gameApp->Game,
		{ 16, 16 }, { 0, 0 }, { 4, 4 }, { 32, 32 });

	S_LOG_INFO("[ WORLD ] Successfully initialized world!");
	world->IsInitialized = true;
}

void WorldLoad(World* world, Game* game)
{
	// TODO hardcoded, would like to figure out nicer way
	// to handle ChunkedTileMap and Game initalized values.
	// TileMapParams struct? or store in Game?
	int screenWidthTiles = game->CurScreenRect.width / 16;
	int screenHeightTiles = game->CurScreenRect.height / 16;
	world->LightMap.Initialize(screenWidthTiles, screenHeightTiles);
	CTileMap::Update(&world->ChunkedTileMap, game);

	world->IsLoaded = true;
	S_LOG_INFO("[ WORLD ] World loaded!");
}

void WorldFree(World* world)
{
	assert(world);
	if (!world->IsInitialized || !world->IsLoaded)
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
	GetGameApp()->NumOfChunksUpdated = 0;

	CTileMap::Update(&world->ChunkedTileMap, game);
	world->LightMap.Update(world);
	world->EntityMgr.Update(game, GetDeltaTime());

	CTileMap::LateUpdateChunk(&world->ChunkedTileMap, game);
	
	//DrawRectangleLinesEx(game->CurScreenRect, 8, ORANGE);

	GetGameApp()->NumOfLoadedChunks = 
		(int)world->ChunkedTileMap.ChunksList.Length;
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
