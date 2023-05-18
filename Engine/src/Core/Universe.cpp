#include "Universe.h"

#include "Game.h"

void UniverseInitialize(Universe* universe, GameApplication* gameApp)
{
	WorldInitialize(&universe->World, gameApp);
}

void UniverseLoad(Universe* universe, GameApplication* gameApp)
{
	WorldLoad(&universe->World, gameApp->Game);
}

void UniverseUnload(Universe* universe, GameApplication* gameApp)
{
	WorldFree(&universe->World);
}

void UniverseUpdate(Universe* universe, Game* game)
{
	universe->UpdateCounter += (float)GetTime();
	if (universe->UpdateCounter > universe->UpdateCounterTarget)
	{
		universe->UpdateCounter = 0.0f;

		++universe->TotalTicks;

		WorldUpdate(&universe->World, game);
	}
}