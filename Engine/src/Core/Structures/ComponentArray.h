#pragma once

#include "Core/Core.h"
#include "Core/Globals.h"
#include "Core/Structures/SList.h"
#include "Core/Structures/SparseSet.h"

template<typename T>
struct ComponentArray
{

	SList<T> Values;
	SparseSet Indices;

	inline uint32_t Size() const
	{
		return Values.Count;
	}

	inline void Initialize()
	{
		Values = {};
		Values.Reserve(1);
		Indices = {};
		Indices.Reserve(0, 1);
	}

	inline T* Add(uint32_t entityId, const T& value)
	{
		uint32_t id = GetId(entityId);
		Indices.Add(id);
		Values.Push(&value);
		return Values.Last();
	}

	inline bool Remove(uint32_t entityId)
	{
		SASSERT(Values.IsAllocated());
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Remove(id);
		if (index != SPARSE_EMPTY_ID)
		{
			Values.RemoveAtFast(index);
			return true;
		}
		return false;
	}

	inline T* Get(uint32_t entityId)
	{
		SASSERT(Values.IsAllocated());
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Get(id);
		if (index == SPARSE_EMPTY_ID) 
			return nullptr;
		return &Values[index];
	}

	inline bool Contains(uint32_t entityId)
	{
		SASSERT(Values.IsAllocated());
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Get(id);
		return (index != SPARSE_EMPTY_ID);
	}

};
