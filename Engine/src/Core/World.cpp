#include "World.h"

#include "Game.h"
#include "ResourceManager.h"

#include <assert.h>

bool WorldInitialize(World* world)
{
	assert(world);

	// TODO call unordered_set constructor without allocating memory
	//new (&world->TileCoordsInLOS) std::unordered_set<Vector2i, PackVector2i>();

	world->TileCoordsInLOS = new std::unordered_set<Vector2i, PackVector2i>();
	world->EntityActionsList.Initialize();
	world->WorldCreatures.Initialize();

	InitializeEntitiesManager(&world->EntitiesManager);

	TestEntities(&world->EntitiesManager);

	assert(&world->TileCoordsInLOS);
	return true;
}

void WorldUpdate(World* world, GameApplication* gameApp)
{
	RenderTileMap(gameApp->Game, &world->MainTileMap);

	for (int i = 0; i < world->WorldCreatures.Length; ++i)
	{
		Creature creature = world->WorldCreatures.Memory[i];
		CreatureUpdate(&creature, gameApp->Game);

		auto find = world->TileCoordsInLOS->find(creature.TilePosition);
		if (find != world->TileCoordsInLOS->end())
		{
			CreatureRender(gameApp->Resources, &creature);
		}
	}
}

void WorldCreateCreature(World* world, Creature* creature)
{
	world->WorldCreatures.Push(creature);
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
