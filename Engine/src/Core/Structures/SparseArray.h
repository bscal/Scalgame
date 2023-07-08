#pragma once

#include "Core/Core.h"

#include "SparseSet.h"
#include "SList.h"

template<typename T>
struct SparseArray
{
	SparseSet Set;
	SList<T> Values;

	_FORCE_INLINE_ bool IsAllocated() const { return Set.IsAllocated() && Values.IsAllocated(); }
	_FORCE_INLINE_ uint32_t GetCount() const { return Values.Count; }
	_FORCE_INLINE_ T* Index(uint32_t idx) { return Values.PeekAt(idx); }

	// maxId - Highest id to store in sparse set, SparseSet::Sparse array capacity will be maxId + 1
	// valueCapacity - Capacity and Count for SparseSet::Dense array and Values array. Using 0 will ignore setting these.
	void Reserve(uint32_t maxId, uint32_t valueCapacity)
	{
		Set.Reserve(maxId, valueCapacity);
		Values.EnsureSize(valueCapacity);
	}

	void Free()
	{
		Set.Free();
		Values.Free();
	}

	T* Add(uint32_t id, const T* value)
	{
		uint32_t idx = Set.Get(id);
		if (idx > Set.MaxValue || idx == SPARSE_EMPTY_ID)
		{
			idx = GetCount();

			Set.Add(id);
			Values.Push(value);
		}
		return Values.PeekAt(idx);
	}

	bool Remove(uint32_t id)
	{
		uint32_t idx = Set.Get(id);
		if (idx > Set.MaxValue || idx == SPARSE_EMPTY_ID)
		{
			return false;
		}
		else
		{
			Set.Remove(idx);
			Values.RemoveAtFast(idx);
			return true;
		}
	}

	T* Get(uint32_t id)
	{
		uint32_t idx = Set.Get(id);
		if (idx > Set.MaxValue || idx == SPARSE_EMPTY_ID)
			return nullptr;
		else
			return Values.PeekAt(idx);
	}

	bool Contains(uint32_t id) const
	{
		uint32_t idx = Set.Get(id);
		return (idx <= Set.MaxValue && idx != SPARSE_EMPTY_ID);
	}
};

