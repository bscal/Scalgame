#pragma once

#include "Core.h"
#include "ChunkedTileMap.h"
#include "TileMapUtils.h"
#include "Entity.h"
#include "Creature.h"
#include "LightSource.h"
#include "Structures/SList.h"

#include <unordered_set>
#include <vector>

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

struct Hash
{
	[[nodiscard]] constexpr size_t operator()(const Vector2i& v) const
	{
		 return SHashMerge(0, v);
	}
};

struct Equals
{
	[[nodiscard]] bool operator()(const Vector2i& l, const Vector2i& r) const
	{
		return l.Equals(r);
	}
};

struct World
{
	CTileMap::ChunkedTileMap ChunkedTileMap;
	Scal::Creature::EntityMgr EntityMgr;
	SList<Action> EntityActionsList;
	std::unordered_set<Vector2i,
		Hash, Equals, SAllocator<Vector2i>> TileCoordsInLOS;
	LightMap LightMap;
	SightMap SightMap;
	TileMgr TileMgr;
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

bool WorldInitialize(World* world, GameApplication* gameApp);
void WorldFree(World* world);
void WorldUpdate(World* world, Game* game);
void LateWorldUpdate(World* world, Game* game);
bool CanMoveToTile(World* world, Vector2i position);
bool WorldIsInBounds(World* world, Vector2i pos);
void TurnEnd(World* world, Game* game, int timeChange);
void AddAction(World* world, Action* action);
void ProcessActions(World* world);