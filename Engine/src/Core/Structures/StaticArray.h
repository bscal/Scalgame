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