#pragma once

#include "Core/Core.h"

#include "Core/Structures/SList.h"
#include "Core/Structures/SLinkedList.h"

#include <math.h>

struct GameApplication;

/*
* Quick and dirty scheduler for fun. 
*/

struct SchedulerTask
{
	void* EntryData;
	int Id;

	virtual bool Process() = 0;
};

struct Scheduler
{
	struct Entry
	{
		SList<SchedulerTask> Tasks;
		int NumCalls;
		int Interval;
		int ListCurrentIndex;
		int ListNextCycleStart;
		float ListProgress;

		void Tick(int currentTick)
		{
			NumCalls = 0;
			if (ListNextCycleStart <= currentTick)
			{
				ListCurrentIndex = 0;
				ListNextCycleStart = currentTick + Interval;
				ListProgress = 0.0f;
			}

			ListProgress += (float)Tasks.Count / (float)Interval;
			uint32_t maxIndex = (uint32_t)fminf((float)Tasks.Count, ceilf(ListProgress));

			SLinkedList<int> indexToRemove = {};
			indexToRemove.Allocator = SMEM_TEMP_ALLOCATOR;

			for (; ListCurrentIndex < maxIndex; ++ListCurrentIndex)
			{
				bool shouldRemove = Tasks[ListCurrentIndex].Process();
				if (shouldRemove)
					indexToRemove.Push(&ListCurrentIndex);
				++NumCalls;
			}

			while (indexToRemove.HasNext())
			{
				int i = indexToRemove.PopValue();
				Tasks.RemoveAt(i);
			}
		}
	};

	SList<Entry> Entries;
	int Tick;
	int NextId;

	void Update()
	{
		for (uint32_t i = 0; i < Entries.Count; ++i)
		{
			Entries[i].Tick(Tick);
		}
		++Tick;
	}

	int64_t AddTask(int interval, const SchedulerTask* task)
	{
		for (uint32_t i = 0; i < Entries.Count; ++i)
		{
			if (Entries[i].Interval == interval)
			{
				Entries[i].Tasks.Push(task);

				int64_t id = NextId++;
				id |= ((int64_t)interval << 32);
				Entries[i].Tasks.Last()->Id = (int)id;
				return id;
			}
		}

		Entry* entry = Entries.PushNew();
		entry->Interval = interval;
		entry->Tasks.Push(task);

		int64_t id = NextId++;
		id |= ((int64_t)interval << 32);
		Entries.Last()->Tasks.Last()->Id = (int)id;
		return id;
	}

	void RemoveTask(int64_t taskId)
	{
		int id = (int)taskId;
		int interval = (int)(taskId >> 32);

		for (uint32_t i = 0; i < Entries.Count; ++i)
		{
			if (Entries[i].Interval == interval)
			{
				for (uint32_t j = 0; j < Entries[i].Tasks.Count; ++j)
				{
					if (Entries[i].Tasks[j].Id == id)
					{
						Entries.RemoveAt(j);
						return;
					}
				}
			}
		}

	}

};
