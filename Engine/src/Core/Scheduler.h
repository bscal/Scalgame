#pragma once

#include "Core.h"
#include "Tile.h"

#include "Structures/SList.h"
#include "Structures/IndexArray.h"

#include <functional>
#include <math.h>

struct ChunkedTileMap;

template<typename Action>
struct DistributedScheduler
{
	IndexArray<Action> Actions;
	int Index;
	int UpdateCount;
	float Interval;
	float UpdateAccumulator;

	void Update(float dt)
	{
		float updatesPerSecond = (float)Actions.Size / Interval;
		float updatesPerFrame = updatesPerSecond * dt;

		UpdateAccumulator = fminf(UpdateAccumulator + updatesPerFrame, (float)Actions.Size);

		UpdateCount = UpdateAccumulator;

		for (int i = 0; i < UpdateCount; ++i)
		{
			Index = (Index + 1) % Actions.Size;

			Action* actionPtr = Actions.At(Index);
			if (actionPtr)
			{
				(*actionPtr)();
			}
		}

		UpdateAccumulator -= (float)UpdateCount;
	}

	uint32_t Add(const Action& action)
	{
		return Actions.Add(&action);
	}

};

struct DistributedTileUpdater
{
	struct TileUpdaterData
	{
		Vector2i Pos;
		void(*OnUpdate)(Vector2i, TileData);
	};

	IndexArray<TileUpdaterData> Actions;
	int Index;
	int UpdateCount;
	float Interval;
	float UpdateAccumulator;

	void Update(ChunkedTileMap* tilemap, float dt);
	uint32_t Add(Vector2i coord, TileData* data);
};

