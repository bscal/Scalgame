#pragma once

#include "Core.h"
#include "Tile.h"

#include "Structures/SList.h"
#include "Structures/IndexArray.h"

#include <functional>
#include <math.h>

struct ChunkedTileMap;
struct TileMapChunk;

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
	constexpr static int COUNT = CHUNK_SIZE;
	constexpr static float INTERVAL = 60.0f / 2.0f;
	constexpr static float UPDATES_PER_SEC = (float)COUNT / INTERVAL;

	int Index;
	float UpdateAccumulator;

	void Update(ChunkedTileMap* tilemap, TileMapChunk* chunk, float dt);
};
