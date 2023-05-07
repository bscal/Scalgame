#pragma once

#include "Core/Core.h"
#include "Core/Globals.h"
#include "Core/Structures/SArray.h"
#include "Core/Structures/SparseSet.h"

struct ComponentArray
{

	SArray Values;
	SparseSet Indices;

	inline uint32_t Size() const
	{
		return Values.Count;
	}

	template<typename T>
	inline void Initialize()
	{
		Values = ArrayCreate((uint8_t)SAllocator::Game, 1, sizeof(T));
		Indices = {};
		Indices.Reserve(0, 1);
	}

	template<typename T>
	inline T* Add(uint32_t entityId, const T& value)
	{
		SASSERT(Values.Memory);
		uint32_t id = GetId(entityId);
		Indices.Add(id);
		ArrayPush(&Values, &value);
		return (T*)ArrayPeekAt(&Values, Values.Count - 1);
	}


	inline bool Remove(uint32_t entityId)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Remove(id);
		if (index != SPARSE_EMPTY_ID)
		{
			ArrayRemoveAt(&Values, index);
			return true;
		}
		return false;
	}

	template<typename T>
	inline T* Get(uint32_t entityId)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Get(id);
		if (index == SPARSE_EMPTY_ID) 
			return nullptr;
		return (T*)ArrayPeekAt(&Values, index);
	}


	template<typename T>
	inline T* GetNotNull(uint32_t entityId)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Get(id);
		return (T*)ArrayPeekAt(&Values, index);
	}

	template<typename T>
	inline T* Index(uint32_t idx)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		return (T*)ArrayPeekAt(&Values, idx);
	}

	inline bool Contains(uint32_t entityId)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entityId);
		uint32_t index = Indices.Get(id);
		return (index != SPARSE_EMPTY_ID);
	}

};
