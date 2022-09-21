#pragma once

#include "Core.h"
#include "TileMap.h"
#include "Creature.h"
#include "Structures/SList.h"

#include <unordered_set>

struct GameApplication;
struct Game;
struct Resources;
struct Action;

struct WorldTime
{
	uint32_t TotalTurns;
	uint32_t TotalDays;
	uint8_t Hour;
	uint8_t Minute;
};

struct World
{
	TileMap MainTileMap;
	SList<Action> EntityActionsList;
	// TODO add own set struct
	std::unordered_set<Vector2i, PackVector2i>* TileCoordsInLOS;
	//SList<Player> WorldPlayers;
	SList<Creature> WorldCreatures;
};

struct Action
{
	void* ActionData;
	void(*ActionFunction)(World* world, Action* action);
	uint32_t ActionId;
	uint32_t EntityId;
	int16_t Cost;
};

bool WorldInitialize(World* world);
void WorldUpdate(World* world, GameApplication* game);

void WorldCreateCreature(World* world, Creature* creature);
void TurnEnd(World* world, Game* game, int timeChange);
void AddAction(World* world, Action* action);
void ProcessActions(World* world);