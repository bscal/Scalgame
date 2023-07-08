#pragma once

#include "Structures/SList.h"
#include "Structures/IndexArray.h"

#include <functional>
#include <math.h>

template<typename Action>
struct DistributedScheduler
{
	IndexArray<Action> Actions;
	int Index;
	float Interval;
	float UpdateAccumulator;

	void DistributedSchedulerUpdate(float dt)
	{
		float updatesPerSecond = (float)Actions.Size / Interval;
		float updatesPerFrame = updatesPerSecond * dt;

		UpdateAccumulator = fminf(UpdateAccumulator + updatesPerFrame, (float)Actions.Size);

		int updateCount = UpdateAccumulator;

		for (int i = 0; i < updateCount; ++i)
		{
			Index = (Index + 1) % Actions.Size;

			Action* actionPtr = Actions.At(Index);
			if (actionPtr)
			{
				(*actionPtr)();
			}
		}

		UpdateAccumulator -= (float)updateCount;
	}

	uint32_t DistributedSchedulerAdd(const Action action)
	{
		SASSERT(scheduler);
		SASSERT(action);
		return Actions.Add(&action);
	}

};
