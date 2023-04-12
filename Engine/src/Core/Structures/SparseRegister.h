#pragma once

#include "Core/Core.h"

#include "ComponentArray.h"
#include "SLinkedList.h"

template<typename T>
struct SparseRegister
{
	ComponentArray<T> ComponentArray;
	SLinkedList<uint32_t> FreeIds;
	uint32_t NextId;

	inline uint32_t Add(const T& value)
	{
		uint32_t id;
		if (FreeIds.HasNext)
			id = FreeIds.PopValue()
		else
			id = NextId++;

		ComponentArray.Add(id, value);
		return id;
	}

	inline void Remove(uint32_t id)
	{
		bool wasRemoved = ComponentArray.Remove(id);
		if (wasRemoved)
			FreeIds.Push(&id);
	}

	inline T* Get(uint32_t id)
	{
		return ComponentArray.Get(id);
	}

	inline bool Contains(uint32_t id)
	{
		return ComponentArray.Contains(id);
	}
};
