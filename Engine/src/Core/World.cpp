#include "World.h"

#include "Game.h"
#include "ResourceManager.h"

#include <assert.h>

World::World() : TileCoordsInLOS()
{
	this->WorldCreatures = {};
	this->EntityActionsList = {};
	this->MainTileMap = {};
}

bool WorldInitialize(World* world)
{
	world->EntityActionsList.Initialize();
	world->WorldCreatures.Initialize();
	return true;
}

void WorldUpdate(World* world, GameApplication* gameApp)
{
	RenderTileMap(gameApp->Game, &world->MainTileMap);

	for (int i = 0; i < world->WorldCreatures.Length; ++i)
	{
		Creature creature = world->WorldCreatures.Memory[i];
		CreatureUpdate(&creature, gameApp->Game);
		CreatureRender(gameApp->Resources, &creature);
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
