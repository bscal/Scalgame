#pragma once

#include "Core.h"
#include "Vector2i.h"


struct Game;
struct World;

struct MonsterType
{

};

struct Monster
{
	uint16_t MonsterTypeId;
};

void SpawnMonster(World* world, Vector2i pos);
void DeleteMonster(World* world, Monster* monster);

void UpdateMonsters(Game* game);
void DrawMonsters(Game* game);
