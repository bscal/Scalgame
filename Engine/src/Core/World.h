#pragma once

#include "Core.h"
#include "ChunkedTileMap.h"

#include "Structures/SList.h"

struct GameApplication;
struct Game;
struct Resources;
struct Action;

struct WorldSettings
{

};

struct WorldTime
{
	uint32_t TotalTurns;
	uint32_t TotalDays;
	uint8_t Hour;
	uint8_t Minute;
};

struct World
{
	ChunkedTileMap ChunkedTileMap;
	
	SList<Action> EntityActionsList;
	bool IsAllocated;
	bool IsLoaded;
};

struct Action
{
	void* ActionData;
	void(*ActionFunction)(World* world, Action* action);
	uint32_t ActionId;
	uint32_t EntityId;
	int16_t Cost;
};

void WorldInitialize(World* world, GameApplication* gameApp);
void WorldLoad(World* world, Game* game);
void WorldFree(World* world);
void WorldUpdate(World* world, Game* game);
void WorldLateUpdate(World* world, Game* game);
bool CanMoveToTile(World* world, Vector2i position);
bool WorldIsInBounds(World* world, Vector2i pos);
void TurnEnd(World* world, Game* game, int timeChange);
void AddAction(World* world, Action* action);
void ProcessActions(World* world);
