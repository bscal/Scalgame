#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

//#include <bitset>
//template <size_t N>
//using BitArray = std::bitset<N>;

struct BitFlags
{
	uint64_t Flag;

	inline bool Get(uint64_t index) const { SASSERT(index < 64); return BitGet(Flag, index); }
	inline void Set(uint64_t index) { SASSERT(index < 64); BitSet(Flag, index); }
	inline void Clear(uint64_t index) { SASSERT(index < 64); BitClear(Flag, index); }
	inline void Toggle(uint64_t index) { SASSERT(index < 64); BitToggle(Flag, index); }
};

template<size_t N>
struct BitList
{
	uint64_t Data[N];

	bool GetBit(uint64_t bit) const;
	void SetBit(uint64_t bit);
	void ClearBit(uint64_t bit);

	inline size_t SizeInBits() const { return N * 64; }
};

template<size_t N>
bool BitList<N>::GetBit(uint64_t bit) const
{
	uint64_t index = bit / SizeInBits();
	uint64_t indexBit = bit % SizeInBits();
	return BitGet(Memory[index], indexBit);
}

template<size_t N>
void BitList<N>::SetBit(uint64_t bit)
{
	uint64_t index = bit / SizeInBits();
	uint64_t indexBit = bit % SizeInBits();
	BitSet(Memory[index], indexBit);
}

template<size_t N>
void BitList<N>::ClearBit(uint64_t bit)
{
	uint64_t index = bit / SizeInBits();
	uint64_t indexBit = bit % SizeInBits();
	BitClear(Memory[index], indexBit);
}
