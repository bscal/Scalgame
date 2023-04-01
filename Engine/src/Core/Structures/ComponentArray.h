#pragma once

#include "Core/Core.h"
#include "Core/Structures/SList.h";
#include "Core/Structures/SparseSet.h"

template<typename T>
struct ComponentArray
{

	SList<T> Values;
	SparseSet Indices;

	inline void Initialize()
	{
		Values.Reserve(1);
		Indices.Reserve(0, 1);
	}

	inline void Add(uint32_t entityId, const T& value)
	{
		Values.Push(*value);
		Indices.Add(entityId);
	}

	inline void Remove(uint32_t entityId)
	{
		SASSERT(Values.IsAllocated());
		SASSERT(Indices.IsAllocated());
		uint32_t index = Indices.Remove(entityId);
		if (index != SPARE_EMPTY_ID)
			Values.RemoveAtFast(index);
	}

	inline T* Get(uint32_t entityId)
	{
		SASSERT(Values.IsAllocated());
		SASSERT(Indices.IsAllocated());
		uint32_t index = Indices.Get(entityId);
		if (index == SPARE_EMPTY_ID) return nullptr;
		return &Values[index];
	}

	inline bool Contains(uint32_t entityId)
	{
		SASSERT(Values.IsAllocated());
		SASSERT(Indices.IsAllocated());
		uint32_t index = Indices.Get(entityId);
		return (index != SPARE_EMPTY_ID);
	}

};
