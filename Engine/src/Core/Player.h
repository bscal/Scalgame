#pragma once

#include "Core.h"
#include "Creature.h"

struct Game;

enum class Direction : uint8_t
{
	North,
	East,
	South,
	West,

	MaxDirections
};

struct Player : public SCreature
{
	void InitializePlayer(World* world);

	void HandleInput(Game* game);
	void UpdatePlayer(Game* game);
};