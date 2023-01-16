#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

struct LinearAllocator
{
	uint64_t Capacity;
	uint64_t Allocated;
	void* Memory;
};

internal inline size_t 
AlignSize(size_t size, size_t alignment)
{
	return (size + (alignment - 1)) & -alignment;
}

void
LinearAllocatorInitialize(LinearAllocator* allocator,
	void* memory, uint64_t capacity)
{
	SASSERT(allocator);
	SASSERT(memory);
	SASSERT(capacity > 0);

	allocator->Capacity = capacity;
	allocator->Allocated = 0;
	allocator->Memory = memory;
}

void*
LinearAllocatorAllocate(LinearAllocator* allocator,
	uint64_t size, uint16_t alignment)
{
	uint64_t alignedSize = AlignSize(size, alignment);
	uint64_t newAllocatedAmount = allocator->Allocated + alignedSize;
	if (newAllocatedAmount > allocator->Capacity)
		return NULL;

	void* ptr = (uint8_t*)(allocator->Memory) + allocator->Allocated;
	allocator->Allocated = newAllocatedAmount;
	return ptr;
}

void
LinearAllocatorReset(LinearAllocator* allocator)
{
	allocator->Allocated = 0;
}