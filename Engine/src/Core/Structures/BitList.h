#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

global_var constexpr size_t WORD_SIZE = sizeof(size_t) * 8;

struct BitList
{
	SMemAllocator Allocator = SMEM_GAME_ALLOCATOR;
	uint64_t* Memory;
	uint32_t Capacity;
	uint32_t CapacityInBits;

	void Reserve(uint64_t bitsCapacity);

	bool GetBit(uint64_t bit) const;
	void SetBit(uint64_t bit);
	void ClearBit(uint64_t bit);
};

void BitList::Reserve(uint64_t bitsCapacity)
{
	uint32_t capacity = bitsCapacity / WORD_SIZE;
	if (capacity > Capacity)
	{
		Capacity = capacity;
		CapacityInBits = Capacity * sizeof(size_t) * 8;
		size_t size = Capacity * sizeof(uint64_t);
		void* tmp = Allocator.Alloc(size);
		SMemCopy(tmp, Memory, size);
		Allocator.Free(Memory);
		Memory = (uint64_t*)tmp;
	}
}

bool BitList::GetBit(uint64_t bit) const
{
	uint32_t index = bit / CapacityInBits;
	uint32_t indexBit = bit % CapacityInBits;
	return BitGet(Memory[index], indexBit);
}

void BitList::SetBit(uint64_t bit)
{
	uint32_t index = bit / CapacityInBits;
	uint32_t indexBit = bit % CapacityInBits;
	BitSet(Memory[index], indexBit);
}

void BitList::ClearBit(uint64_t bit)
{
	uint32_t index = bit / CapacityInBits;
	uint32_t indexBit = bit % CapacityInBits;
	BitClear(Memory[index], indexBit);
}
