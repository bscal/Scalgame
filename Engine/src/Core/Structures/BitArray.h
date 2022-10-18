#pragma once

#include "Core/Core.h"

template<size_t SizeInInt64>
struct BitArray
{
	uint64_t Data[SizeInInt64];

	bool GetBit(uint32_t index) const;
	void SetBit(uint32_t index, bool value);
	void ClearBit(uint32_t index);
};

template<size_t SizeInInt64>
bool BitArray<SizeInInt64>::GetBit(uint32_t index) const
{
	size_t index = (size_t)index / SizeInInt64;
	size_t bit = (size_t)index % SizeInInt64;
	uint64_t value = Data[index];
	return BitGet(value, bit);
}

template<size_t SizeInInt64>
void BitArray<SizeInInt64>::SetBit(uint32_t index, bool value)
{
	size_t index = (size_t)index / SizeInInt64;
	size_t bit = (size_t)index % SizeInInt64;
	Data[index] = BitSet(Data[index], bit);
}

template<size_t SizeInInt64>
void BitArray<SizeInInt64>::ClearBit(uint32_t index)
{
	for (int i = 0; i < sizeof(Data); ++i)
	{
		Data[i] = 0x00;
	}
}