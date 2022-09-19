#pragma once

#include "Core.h"

struct Game;

struct Creature
{
	uint32_t EntityId;
};

bool CreatureInitialize(Creature* creature);

void CreatureUpdate(Creature* creature, Game* game);
void CreatureRender(Creature* creature);