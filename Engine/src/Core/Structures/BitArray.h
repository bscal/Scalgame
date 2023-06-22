#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

struct BitFlags
{
	uint64_t Flag;

	inline bool Get(uint64_t index) const { SASSERT(index < 64); return BitGet(Flag, index); }
	inline void Set(uint64_t index) { SASSERT(index < 64); BitSet(Flag, index); }
	inline void Clear(uint64_t index) { SASSERT(index < 64); BitClear(Flag, index); }
	inline void Toggle(uint64_t index) { SASSERT(index < 64); BitToggle(Flag, index); }
};

template<uint64_t Size>
struct BitArray
{
	uint64_t Memory[Size];

	inline bool Get(uint64_t bit) const
	{
		SASSERT(bit < Size * 64);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		return BitGet(Memory[index], indexBit);
	}

	inline void Set(uint64_t bit)
	{
		SASSERT(bit < Size * 64);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		BitSet(Memory[index], indexBit);
	}

	inline void Clear(uint64_t bit)
	{
		SASSERT(bit < Size * 64);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		BitClear(Memory[index], indexBit);
	}

	inline void Toggle(uint64_t bit)
	{
		SASSERT(bit < Size * 64);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		BitToggle(Memory[index], indexBit);
	}
};

struct BitList
{

	uint64_t* Memory;
	uint32_t Capacity;
	uint32_t SizeInBits;
	SAllocator Allocator;

	void Alloc(SAllocator allocator, uint32_t capacity)
	{
		size_t oldSize = Capacity * sizeof(uint64_t);

		Capacity = (capacity == 0) ? 1 : capacity;
		size_t newSize = Capacity * sizeof(uint64_t);

		Allocator = allocator;
		SizeInBits = Capacity * 64;
		Memory = (uint64_t*)SRealloc(Allocator, Memory, oldSize, newSize, MemoryTag::Arrays);
	}

	void Free()
	{
		if (Memory)
		{
			size_t memSize = Capacity * sizeof(uint64_t);
			SFree(Allocator, Memory, memSize, MemoryTag::Arrays);
			Memory = nullptr;
		}
	}

	_FORCE_INLINE_ int GetBit(uint64_t bit) const
	{
		if (bit >= SizeInBits)
			return 0;

		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		return (int)BitGet(Memory[index], indexBit);
	}

	_FORCE_INLINE_ int GetThenClearBit(uint64_t bit)
	{
		if (bit >= SizeInBits)
			return 0;

		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		int val = (int)BitGet(Memory[index], indexBit);
		BitClear(Memory[index], indexBit);
		return val;
	}

	_FORCE_INLINE_ void SetBit(uint64_t bit)
	{
		if (bit >= SizeInBits)
			Alloc(Allocator, Capacity * 2);

		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitSet(Memory[index], indexBit);
	}

	_FORCE_INLINE_ void ClearBit(uint64_t bit)
	{
		if (bit >= SizeInBits)
			return;

		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitClear(Memory[index], indexBit);
	}

	_FORCE_INLINE_ void Toggle(uint64_t bit)
	{
		if (bit >= SizeInBits)
			Alloc(Allocator, Capacity * 2);

		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitToggle(Memory[index], indexBit);
	}
};
