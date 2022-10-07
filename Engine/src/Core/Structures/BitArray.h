#pragma once

#include "Core/Core.h"

template<uint32_t FieldSizeInBytes>
struct BitArray
{
	bool Data[FieldSizeInBytes];

	bool GetBit(uint32_t index) const;
	void SetBit(uint32_t index, bool value);
	void ClearBit(uint32_t index);
};

template<uint32_t FieldSizeInBytes>
bool BitArray<FieldSizeInBytes>::GetBit(uint32_t index) const
{
	return Data[index];
}

template<uint32_t FieldSizeInBytes>
void BitArray<FieldSizeInBytes>::SetBit(uint32_t index, bool value)
{
	Data[index] = value;
}

template<uint32_t FieldSizeInBytes>
void BitArray<FieldSizeInBytes>::ClearBit(uint32_t index)
{
	Data[index] = 0;
}