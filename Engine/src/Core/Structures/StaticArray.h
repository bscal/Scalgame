#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#define BoundsCheck(data, idx, size) \
SASSERT(idx < size); \
T* dataPtr = (T*)data; \
T* dataOffsetPtr = (T*)(data) + idx; \
SASSERT(dataOffsetPtr >= (T*)dataPtr); \
SASSERT(dataOffsetPtr < (T*)dataPtr + size); \

template<typename T, size_t ElementCount>
struct SStaticArray
{
	T Data[ElementCount];

	inline const T& operator[](size_t idx) const
	{
		BoundsCheck(Data, idx, ElementCount);
		return Data[idx];
	}

	inline T& operator[](size_t idx)
	{
		BoundsCheck(Data, idx, ElementCount);
		return Data[idx];
	}

	inline void Fill(const T& value)
	{
		for (size_t i = 0; i < ElementCount; ++i)
		{
			SMemCopy(&Data[i], &value, sizeof(T));
		}
	}

	inline size_t Size(void) const
	{
		return ElementCount;
	}
};

template <typename T, size_t N>
using StaticArray = SStaticArray<T, N>;

template<typename T>
struct ConstArray
{
	T* Memory;
	uint32_t Size;
	SAllocator Allocator;

	void Initialize(uint32_t capacity, SAllocator allocator)
	{
		SASSERT(!Memory);
		SASSERT(Size == 0);

		Size = capacity;
		Allocator allocator;
		Memory = SAlloc(Allocator, Size * sizeof(T), MemoryTag::Arrays);
	}

	T* operator[](uint32_t idx)
	{
		SASSERT(Memory);
		SASSERT(idx < Size);
		return &Memory[idx];
	}
};