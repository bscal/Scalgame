#pragma once

#include "Core.h"

struct Game;
struct Resources;

struct CreatureType
{
	Rectangle SpriteTextureRect;
	const void* TypeData;
	const char* CreatureName;
	uint32_t EntityId;
	uint32_t MaxHealth;
	uint32_t MaxMana;
	uint16_t Level;
	uint16_t MaxLevel;
};

struct Creature
{
	Rectangle SpriteTextureRect;
	Vector2 Position;
	Vector2i TilePosition;
	uint32_t EntityId;
	uint32_t Health;
	uint32_t MaxHealth;
	uint32_t Mana;
	uint32_t MaxMana;
	uint16_t Level;
	uint8_t Difficulty;
};

global_var uint32_t EntityIdHandler = 0;

global_var const CreatureType ZOMBIE =
{
	{ 0, 16, 16, 16 },
	0,
	"Zombie",
	EntityIdHandler++,
	10,
	0,
	1,
	2
};

bool CreatureInitialize(Creature* creature, const CreatureType* creatureType);

void CreatureUpdate(Creature* creature, Game* game);
void CreatureRender(Resources* resources, Creature* creature);

void CreatureAIPathfind(Creature* creature, Game* game);
void CreatureMove(Creature* creature, int tileX, int tileY);