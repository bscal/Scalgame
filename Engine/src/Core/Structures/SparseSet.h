#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#define SPARSE_EMPTY_ID UINT32_MAX

#define SparseBoundsCheck(ptr, cap, index) SASSERT(ptr + index >= ptr); SASSERT(ptr + index < ptr + cap)

struct SparseSet
{
	SAllocator::Type Allocator;
	uint32_t* Sparse;
	uint32_t* Dense;
	uint32_t Count;
	uint32_t SparseCapacity;
	uint32_t DenseCapacity;
	uint32_t MaxValue;

	inline bool IsAllocated() { return (Sparse && Dense); }

	void Reserve(uint32_t maxValue, uint32_t size)
	{
		ResizeSparse(maxValue + 1);
		ResizeDense(size);
	}

	void Free()
	{
		SFree(Allocator, Sparse, SparseCapacity * sizeof(uint32_t), MemoryTag::Lists);
		Sparse = nullptr;
		SparseCapacity = 0;
		SFree(Allocator, Dense, DenseCapacity * sizeof(uint32_t), MemoryTag::Lists);
		Dense = nullptr;
		DenseCapacity = 0;
		Count = 0;
		MaxValue = 0;
	}

	void Clear()
	{
		SMemSet(Sparse, SPARSE_EMPTY_ID, SparseCapacity * sizeof(uint32_t));
		Count = 0;
	}

	void ResizeSparse(uint32_t capacity)
	{
		SASSERT(capacity > SparseCapacity);
		if (capacity <= SparseCapacity) return;

		size_t oldSize = SparseCapacity * sizeof(uint32_t);
		size_t newSize = capacity * sizeof(uint32_t);
		Sparse = (uint32_t*)SRealloc(Allocator, Sparse, oldSize, newSize, MemoryTag::Lists);
		SASSERT(Sparse);
		SMemSet((void*)(Sparse + Count), SPARSE_EMPTY_ID, newSize - oldSize);

		SparseCapacity = capacity;
		MaxValue = capacity - 1;
	}

	void ResizeDense(uint32_t capacity)
	{
		SASSERT(capacity > DenseCapacity);
		if (capacity <= DenseCapacity) return;

		size_t oldSize = DenseCapacity * sizeof(uint32_t);
		size_t newSize = capacity * sizeof(uint32_t);
		Dense = (uint32_t*)SRealloc(Allocator, Dense, oldSize, newSize, MemoryTag::Lists);
		SASSERT(Dense);

		DenseCapacity = capacity;
	}

	void Add(uint32_t id)
	{
		if (id >= SparseCapacity)
			ResizeSparse(id + 1);

		if (Count >= DenseCapacity)
			ResizeDense(Count + 1);

		uint32_t index = Count;

		SparseBoundsCheck(Dense, DenseCapacity, index);
		Dense[index] = id;

		SparseBoundsCheck(Sparse, SparseCapacity, id);
		Sparse[id] = index;

		++Count;
	}

	uint32_t Remove(uint32_t id)
	{
		SASSERT(id <= MaxValue);

		if (id >= SparseCapacity) 
			return SPARSE_EMPTY_ID;

		SparseBoundsCheck(Sparse, SparseCapacity, id);
		uint32_t index = Sparse[id];
		if (index < Count - 1)
		{
			SparseBoundsCheck(Dense, DenseCapacity, index);
			Dense[index] = Dense[Count - 1];
			uint32_t moved = Dense[index];
			SparseBoundsCheck(Sparse, SparseCapacity, moved);
			Sparse[moved] = index;
		}
		Sparse[id] = SPARSE_EMPTY_ID;
		--Count;
		return index;
	}

	uint32_t Get(uint32_t id) const
	{
		SASSERT(id <= MaxValue);
		SparseBoundsCheck(Sparse, SparseCapacity, id);
		return Sparse[id];
	}
};

inline bool TestSparseSet()
{
	SparseSet arr = {};
	
	arr.Reserve(2, 2);
	SASSERT(arr.IsAllocated());
	SASSERT(arr.SparseCapacity == 3);
	SASSERT(arr.DenseCapacity == 2);
	SASSERT(arr.MaxValue == 2);
	arr.Add(2);
	arr.Add(0);

	auto i0 = arr.Get(0);
	SASSERT(i0 == 1);
	auto i1 = arr.Get(2);
	SASSERT(i1 == 0);
	auto i2 = arr.Get(1);
	SASSERT(i2 == SPARSE_EMPTY_ID);

	arr.Add(10);
	SASSERT(arr.SparseCapacity == 11);
	SASSERT(arr.DenseCapacity == 3);
	SASSERT(arr.MaxValue == 10);

	auto i3 = arr.Get(7);
	SASSERT(i3 == SPARSE_EMPTY_ID);

	auto i4 = arr.Get(10);
	SASSERT(i4 == 2);

	auto i5 = arr.Get(0);
	SASSERT(i5 == 1);

	arr.Remove(0);

	auto i6 = arr.Get(0);
	SASSERT(i6 == SPARSE_EMPTY_ID);

	auto i7 = arr.Get(10);
	SASSERT(i7 == 1);

	return true;
}