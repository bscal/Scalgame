#include "World.h"

#include "Game.h"
#include "ResourceManager.h"

#include <assert.h>

bool WorldInitialize(World* world, TileSet* tileSet)
{
	assert(world);

	// TODO call unordered_set constructor without allocating memory
	//auto c = CALL_CONSTRUCTOR(&world->TileCoordsInLOS)
	//	std::unordered_set<Vector2i, Hash, Equals>();

	world->TileCoordsInLOS = new std::unordered_set<Vector2i, Hash, Equals>();
	world->EntityActionsList.Initialize();

	ChunkedTileMap::Initialize(&world->TTileMap, tileSet,
		{ 0, 0 }, { 16, 16 }, { 64, 64 });
	ChunkedTileMap::Create(&world->TTileMap, 2, 2);

	InitializeEntitiesManager(&world->EntitiesManager);

	TestEntities(&world->EntitiesManager);

	// TODO
	world->IsLoaded = true;
	assert(&world->TileCoordsInLOS);
	return true;
}

void WorldFree(World* world)
{
	if (!world)
	{
		S_LOG_ERR("Trying to unload a null world");
		return;
	}

	if (world->IsLoaded)
	{

	}
	world->IsLoaded = false;

	delete world->TileCoordsInLOS;
	ChunkedTileMap::Free(&world->TTileMap);
	world->EntityActionsList.Free();
}

void WorldUpdate(World* world, GameApplication* gameApp)
{
	RenderTileMap(gameApp->Game, &world->MainTileMap);

	ChunkedTileMap::Update(&world->TTileMap, gameApp);

	UpdateSystems(&world->EntitiesManager, gameApp);

	//for (int i = 0; i < world->WorldCreatures.Length; ++i)
	//{
	//	Creature creature = world->WorldCreatures.Memory[i];
	//	CreatureUpdate(&creature, gameApp->Game);

	//	auto find = world->TileCoordsInLOS->find(creature.TilePosition);
	//	if (find != world->TileCoordsInLOS->end())
	//	{
	//		CreatureRender(gameApp->Resources, &creature);
	//	}
	//}
}

void MoveActor(World* world, Vector2 position)
{

}

void MoveActorTile(World* world, Vector2i position)
{

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
