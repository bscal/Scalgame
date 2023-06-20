#pragma once

#include "Core/Core.h"

#include "SList.h"
#include "SLinkedList.h"
#include "BitArray.h"

// A dynamic array that does not reorder elements which are removed.
// LinkedList keep track of unused id, BitList keep track of tombstones.
template<typename T>
struct IndexArray
{
	SList<T> Data;
	SLinkedList<uint32_t> FreeList;
	BitList IndexOccupied;
	uint32_t Size;

	uint32_t Add(const T* value)
	{
		SASSERT(value);

		uint32_t idx;
		if (FreeList.HasNext())
		{
			idx = FreeList.PopValue();
			Data[idx] = *value;
		}
		else
		{
			idx = Data.Count;
			Data.Push(value);
		}
		IndexOccupied.SetBit(idx);
		++Size;
		return idx;
	}

	void Remove(uint32_t idx)
	{
		SASSERT(Data.Capacity > idx);
		IndexOccupied.ClearBit(idx);
		FreeList.Push(&idx);
		--Size;
	}

	// Removes idx, but returns pointer to Data if slot was in use.
	// Memory in Data is not clear so pointer is valid, useful if you
	// need to use the contents to do more cleanup.
	T* RemoveAndGetPtr(uint32_t idx)
	{
		SASSERT(Data.Capacity > idx);
		int slotFull = IndexOccupied.GetThenClearBit(idx);
		if (slotFull)
		{
			T* copy = &Data[idx];
			FreeList.Push(&idx);
			--Size;
			return copy;
		}
		return nullptr;
	}

	T* At(uint32_t idx)
	{
		SASSERT(Data.Capacity > idx);
		if (IndexOccupied.GetBit(idx) == 0)
			return nullptr;

		return &Data[idx];
	}

};

inline int TestIndexArray()
{
	IndexArray<Vector2> iArr = {};

	Vector2 v0 = { 5, 5 };

	uint32_t i0 = iArr.Add(&v0);

	Vector2 v1 = { 6, 6 };
	uint32_t i1 = iArr.Add(&v1);
	SASSERT(i1 == 1);

	Vector2* r0 = iArr.At(i0);
	SASSERT(r0->x == v0.x);

	Vector2* r1 = iArr.At(i1);
	SASSERT(r1->x == v1.x);

	SASSERT(iArr.Size == 2);

	iArr.Remove(i0);

	SASSERT(iArr.Size == 1);
	SASSERT(!iArr.IndexOccupied.GetBit(0));
	SASSERT(iArr.IndexOccupied.GetBit(1));
	SASSERT(!iArr.IndexOccupied.GetBit(2));

	Vector2 v2 = { 11, 11 };
	uint32_t i2 = iArr.Add(&v2);

	SASSERT(i2 == 0);
	SASSERT(iArr.Size == 2);
	SASSERT(iArr.IndexOccupied.GetBit(0));

	Vector2* r2 = iArr.At(i2);
	SASSERT(r2->x == v2.x);

	return 1;
}
