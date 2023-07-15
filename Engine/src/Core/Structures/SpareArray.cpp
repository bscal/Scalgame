#include "SparseArray.h"

#include "Core/SMemory.h"

void SparseArray::SetIdArraySize(uint16_t capacity)
{
	uint16_t oldCap = IdsCapacity;
	IdsCapacity = capacity;
	Ids = (uint16_t*)SRealloc(SAllocator::Game, Ids, sizeof(uint16_t) * IdsCapacity, sizeof(uint16_t) * oldCap, MemoryTag::SparseSet);
	for (uint16_t i = oldCap; i < IdsCapacity; ++i)
	{
		Ids[i] = EMPTY_ID;
	}
}

void SparseArray::SetValueArraySize(uint16_t capacity, uint32_t stride)
{
	uint16_t oldCap = IndicesCapacity;

	IndicesCapacity = capacity;
	LastStride = stride;
	Values = SRealloc(SAllocator::Game, Values, stride * IndicesCapacity, stride * oldCap, MemoryTag::SparseSet);
	Indices = (uint16_t*)SRealloc(SAllocator::Game, Indices, sizeof(uint16_t) * IndicesCapacity, sizeof(uint16_t) * oldCap, MemoryTag::SparseSet);
}

void SparseArray::Free(uint16_t stride)
{
	SFree(SAllocator::Game, Ids, sizeof(uint16_t) * IdsCapacity, MemoryTag::SparseSet);
	SFree(SAllocator::Game, Values, sizeof(uint16_t) * IndicesCapacity, MemoryTag::SparseSet);
	SFree(SAllocator::Game, Indices, LastStride * IndicesCapacity, MemoryTag::SparseSet);
}

uint16_t SparseArray::Add(uint16_t id, const void* value, uint32_t stride)
{
	SASSERT(value);

	if (id >= IdsCapacity)
	{
		SetIdArraySize(id + 1);
	}

	uint16_t idx = Count;

	if (idx >= IndicesCapacity)
	{
		SetValueArraySize(idx, stride);
	}

	++Count;

	Ids[id] = idx;
	Indices[idx] = id;

	void* dst = SparseArrayOffset(Values, stride, idx);
	SMemCopy(dst, value, stride);

	return idx;
}

void* SparseArray::Get(uint16_t id, uint32_t stride)
{
	if (id >= IdsCapacity)
		return nullptr;

	uint16_t idx = Ids[id];

	if (idx == EMPTY_ID)
		return nullptr;

	SASSERT(idx >= IndicesCapacity);

	void* val = SparseArrayOffset(Values, stride, idx);
	SASSERT(val);
	return val;
}

uint16_t SparseArray::Remove(uint16_t id, uint32_t stride)
{
	if (id >= IdsCapacity)
		return EMPTY_ID;

	uint16_t idx = Ids[id];

	if (idx == EMPTY_ID)
		return EMPTY_ID;

	SASSERT(idx >= IndicesCapacity);

	uint16_t lastIdx = (Count > 0) ? Count - 1 : Count;

	uint16_t last = Indices[lastIdx];
	uint16_t lastId = Ids[last];

	Ids[last] = idx;
	Indices[idx] = lastId;

	Ids[idx] = EMPTY_ID;

	void* dst = SparseArrayOffset(Values, stride, idx);
	void* src = SparseArrayOffset(Values, stride, lastIdx);

	SMemCopy(dst, src, stride);

	return idx;
}

bool SparseArray::Contains(uint16_t id) const
{
	if (id >= IdsCapacity)
		return false;

	uint16_t idx = Ids[id];

	return (idx != EMPTY_ID);
}