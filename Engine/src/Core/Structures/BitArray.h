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

constexpr static uint64_t
CeilBitsToU64(uint64_t v)
{
	float num = (float)v / 64.0f;
	return ((uint64_t)num == num) ? (uint64_t)num : (uint64_t)num + ((num > 0) ? 1 : 0);
}

template<uint64_t SizeInBits>
struct BitArray
{
	constexpr static uint64_t ElementCount = CeilBitsToU64(SizeInBits);

	uint64_t Memory[ElementCount];

	_FORCE_INLINE_ bool Get(uint64_t bit) const
	{
		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		return BitGet(Memory[index], indexBit);
	}

	_FORCE_INLINE_ void Set(uint64_t bit)
	{
		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitSet(Memory[index], indexBit);
	}

	_FORCE_INLINE_ void Clear(uint64_t bit)
	{
		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitClear(Memory[index], indexBit);
	}

	_FORCE_INLINE_ void Toggle(uint64_t bit)
	{
		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitToggle(Memory[index], indexBit);
	}
};

// Dynamic, growable array of u64's
struct BitList
{
	uint64_t* Memory;
	uint32_t Capacity;
	uint32_t SizeInBits;
	SAllocator Allocator;

	void Alloc(SAllocator allocator, uint64_t bitsNeeded)
	{
		size_t oldSize = (size_t)Capacity * sizeof(uint64_t);

		uint64_t capacity = CeilBitsToU64(bitsNeeded);
		if (capacity == 0)
			capacity = 1;
		
		Capacity = (uint32_t)capacity;

		size_t newSize = (size_t)Capacity * sizeof(uint64_t);

		Allocator = allocator;
		SizeInBits = Capacity * 64;
		Memory = (uint64_t*)SRealloc(Allocator, Memory, oldSize, newSize, MemoryTag::Arrays);
		SASSERT(Memory);
	}

	void Free()
	{
		if (Memory)
		{
			SFree(Allocator, Memory, (size_t)Capacity * sizeof(uint64_t), MemoryTag::Arrays);
			Memory = nullptr;
			Capacity = 0;
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
		Memory[index] = BitClear(Memory[index], indexBit);
		return val;
	}

	_FORCE_INLINE_ void SetBit(uint64_t bit)
	{
		if (bit >= SizeInBits)
			Alloc(Allocator, bit);

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
			Alloc(Allocator, bit);

		SASSERT(bit < SizeInBits);
		uint64_t index = bit / 64;
		uint64_t indexBit = bit % 64;
		Memory[index] = BitToggle(Memory[index], indexBit);
	}
};
