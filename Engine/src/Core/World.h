#pragma once

#include "TileMap.h"
#include "Creature.h"
#include "Player.h"
#include "Structures/SList.h"

struct GameApplication;
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

	Player Player;
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

void TurnEnd(World* world, Game* game, int timeChange);
void AddAction(World* world, Action* action);
void ProcessActions(World* world);