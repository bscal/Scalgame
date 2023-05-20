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
	SAllocator Allocator;
	uint32_t Size;

	inline void Initialize(SAllocator allocator, uint32_t size)
	{
		Allocator = allocator;
		Size = size;

		size_t memSize = Size * sizeof(uint64_t);
		Memory = (uint64_t*)SAlloc(Allocator, memSize, MemoryTag::Arrays);
	}

	inline void Free()
	{
		if (Memory)
		{
			size_t memSize = Size * sizeof(uint64_t);
			SFree(Allocator, Memory, memSize, MemoryTag::Arrays);
			Memory = nullptr;
		}
	}

	inline bool GetBit(uint64_t bit) const
	{
		SASSERT(bit < Size * 64);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		return BitGet(Memory[index], indexBit);
	}

	void SetBit(uint64_t bit)
	{
		SASSERT(bit < Size * 64);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		BitSet(Memory[index], indexBit);
	}


	void ClearBit(uint64_t bit)
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
