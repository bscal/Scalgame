#pragma once

#include "World.h"

struct GameApplication;
struct Game;

struct Universe
{
	World World;

	uint64_t TotalTicks;
	float UpdateCounter;
	float UpdateCounterTarget;
};

void UniverseInitialize(Universe* universe, GameApplication* gameApp);

void UniverseLoad(Universe* universe, GameApplication* gameApp);
void UniverseUnload(Universe* universe, GameApplication* gameApp);

void UniverseUpdate(Universe* universe, Game* game);
