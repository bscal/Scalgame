#include "World.h"

#include "Game.h"

bool WorldInitialize(World* world)
{
	world->EntityActionsList.Initialize();
	return true;
}

void WorldUpdate(World* world, GameApplication* gameApp)
{
	RenderTileMap(gameApp->Game, &world->MainTileMap);

	for (int i = 0; i < world->WorldCreatures.Length; ++i)
	{
		Creature creature = world->WorldCreatures.Memory[i];
		CreatureUpdate(&creature, gameApp->Game);
		CreatureRender(&creature);
	}

	UpdatePlayer(gameApp, &world->Player);
	RenderPlayer(gameApp, &world->Player);
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