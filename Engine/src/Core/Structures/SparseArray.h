#pragma once

#include "Core/Core.h"
#include "Core/Structures/SList.h"
#include "Core/Structures/SLinkedList.h"
#include "Core/Structures/SparseSet.h"

template<typename T>
struct SparseArray
{
	SparseSet Indices;
	SList<T> Values;
	SLinkedList<uint32_t> FreeList;
	uint32_t NextId;

	_FORCE_INLINE_ uint32_t Size() const { return Values.Count; }

	_FORCE_INLINE_ uint32_t NewId()
	{
		uint32_t id;
		if (FreeList.HasNext())
			id = FreeList.PopValue();
		else
			id = NextId++;
		return id;
	}

	T* Add(uint32_t id, const T* value)
	{
		SASSERT(value);
		uint32_t idx = Indices.Get(id);
		if (idx == SPARSE_EMPTY_ID)
		{
			idx = Values.Count;
			Indices.Add(id);
			Values.Push(value);
		}
		return Values.PeekAt(idx);
	}

	bool Remove(uint32_t id)
	{
		uint32_t idx = Indices.Remove(id);
		if (idx != SPARSE_EMPTY_ID)
		{
			Values.RemoveAtFast(idx);
			FreeList.Push(&id);
			return true;
		}
		else
		{
			return false;
		}
	}

	_FORCE_INLINE_ T* Get(uint32_t id)
	{
		uint32_t idx = Indices.Get(id);
		if (idx == SPARSE_EMPTY_ID)
			return nullptr;
		else
			return Values.PeekAt(idx);
	}

	_FORCE_INLINE_ T* Index(uint32_t idx)
	{
		SASSERT(idx < Values.Count);
		SASSERT(Values.Memory);
		return Values.PeekAt(idx);
	}
};
