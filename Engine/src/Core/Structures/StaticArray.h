#pragma once

#define BoundsCheck(data, idx, size) \
SASSERT(idx < size); \
T* dataPtr = data; \
T* dataOffsetPtr = (T*)(data) + idx; \
SASSERT(dataOffsetPtr >= dataPtr); \
SASSERT(dataOffsetPtr < dataPtr + size); \

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
			memcpy(&Data[i], &value, sizeof(T));
		}
	}

	inline size_t Size(void) const
	{
		return ElementCount;
	}
};

template <typename T, size_t N>
using StaticArray = SStaticArray<T, N>;