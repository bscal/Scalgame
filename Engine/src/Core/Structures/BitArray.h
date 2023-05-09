#pragma once

#include "Core/Core.h"

template<uint64_t Size>
struct BitArray
{
	uint64_t Memory[Size];

	bool GetBit(uint64_t bit) const;
	void SetBit(uint64_t bit);
	void ClearBit(uint64_t bit);
	void Clear();
};

template<uint64_t Size>
bool BitArray<Size>::GetBit(uint64_t bit) const
{
	uint64_t index = bit / sizeof(uint64_t);
	uint64_t indexBit = bit % sizeof(uint64_t);
	return BitGet(Memory[index], indexBit);
}

template<uint64_t Size>
void BitArray<Size>::SetBit(uint64_t bit)
{
	uint64_t index = bit / sizeof(uint64_t);
	uint64_t indexBit = bit % sizeof(uint64_t);
	BitSet(Memory[index], indexBit);
}

template<uint64_t Size>
void BitArray<Size>::ClearBit(uint64_t bit)
{
	uint64_t index = bit / sizeof(uint64_t);
	uint64_t indexBit = bit % sizeof(uint64_t);
	BitClear(Memory[index], indexBit);
}

template<uint64_t Size>
void BitArray<Size>::Clear()
{
	for (uint64_t i = 0; i < Size; ++i)
	{
		Memory[index] = 0;
	}
}