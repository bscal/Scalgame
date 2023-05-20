#pragma once

#include "Core/Core.h"
#include "Core/Structures/SArray.h"
#include "Core/Structures/SparseSet.h"

typedef void(*OnAdd)(uint32_t, void*);
typedef void(*OnRemove)(uint32_t, void*);

struct ComponentArray
{

	SArray Values;
	SparseSet Indices;
	OnAdd AddCallback;
	OnRemove RemoveCallback;

	inline void Initialize(uint32_t defaultCap, size_t stride, OnAdd addCb, OnRemove removeCb)
	{
		Values = ArrayCreate((uint8_t)SAllocator::Game, defaultCap, stride);
		SMemSet(&Indices, 0, sizeof(SparseSet));
		Indices.Reserve(0, defaultCap);
		AddCallback = addCb;
		RemoveCallback = removeCb;
	}

	template<typename T>
	inline T* Add(uint32_t entity, const T& value)
	{
		SASSERT(Values.Memory);
		uint32_t id = GetId(entity);
		if (id > Indices.MaxValue || Indices.Get(id) == SPARSE_EMPTY_ID)
		{
			uint32_t idx = Values.Count;

			Indices.Add(id);
			ArrayPush(&Values, &value);

			T* valuePtr = (T*)ArrayPeekAt(&Values, idx);

			if (AddCallback)
				AddCallback(entity, valuePtr);

			return valuePtr;
		}
		else
		{
			uint32_t idx = Indices.Get(id);
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
			if (RemoveCallback)
				RemoveCallback(entity, ArrayPeekAt(&Values, idx));

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

	inline uint32_t Size() const
	{
		return Values.Count;
	}
};
