#pragma once

#include "Core/Core.h"
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
	inline T* Add(uint32_t entity, const T& value)
	{
		SASSERT(Values.Memory);
		uint32_t id = GetId(entity);
		uint32_t idx = Indices.Get(id);
		if (idx == SPARSE_EMPTY_ID)
		{
			Indices.Add(id);
			ArrayPush(&Values, &value);
			return (T*)ArrayPeekAt(&Values, Values.Count - 1);
		}
		else
		{
			return (T*)ArrayPeekAt(&Values, idx);
		}
	}


	inline bool Remove(uint32_t entity)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entity);
		uint32_t idx = Indices.Remove(id);
		if (idx != SPARSE_EMPTY_ID)
		{
			ArrayRemoveAt(&Values, idx);
			return true;
		}
		else
		{
			return false;
		}
	}

	template<typename T>
	inline T* Get(uint32_t entity)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entity);
		uint32_t idx = Indices.Get(id);
		if (idx == SPARSE_EMPTY_ID)
			return nullptr;
		else
			return (T*)ArrayPeekAt(&Values, idx);
	}


	template<typename T>
	inline T* GetNotNull(uint32_t entity)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entity);
		uint32_t idx = Indices.Get(id);
		return (T*)ArrayPeekAt(&Values, idx);
	}

	template<typename T>
	inline T* Index(uint32_t idx)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		return (T*)ArrayPeekAt(&Values, idx);
	}

	inline bool Contains(uint32_t entity)
	{
		SASSERT(Values.Memory);
		SASSERT(Indices.IsAllocated());
		uint32_t id = GetId(entity);
		uint32_t idx = Indices.Get(id);
		return (idx != SPARSE_EMPTY_ID);
	}

};
