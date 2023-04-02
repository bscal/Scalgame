#pragma once

#include "Core.h"
#include "Creature.h"

struct Game;

struct Player : public SCreature
{
	void InitializePlayer(World* world);

	void HandleInput(Game* game);
	void UpdatePlayer(Game* game);
};