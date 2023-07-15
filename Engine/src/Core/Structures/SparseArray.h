#pragma once

#include "Core/Core.h"

#define SparseArrayOffset(ptr, stride, idx) (((uint8_t*)ptr) + (stride * idx))
#define SparseArrayIndex(idx, T) (((T*)Values)[idx])
#define SparseArrayGet(arr, idx, stride, T) ((T*)Arr.Get(idx, stride))

struct SparseArray
{
	constexpr static uint16_t EMPTY_ID = UINT16_MAX;

	void* Values;
	uint16_t* Ids;
	uint16_t* Indices;
	uint16_t IdsCapacity;
	uint16_t IndicesCapacity;
	uint16_t Count;
	uint16_t LastStride;

	void SetIdArraySize(uint16_t capacity);

	void SetValueArraySize(uint16_t capacity, uint32_t stride);

	void Free(uint16_t stride);

	uint16_t Add(uint16_t id, const void* value, uint32_t stride);

	void* Get(uint16_t id, uint32_t stride);

	uint16_t Remove(uint16_t id, uint32_t stride);

	bool Contains(uint16_t id) const;

	_FORCE_INLINE_ uint16_t IsAllocated() const noexcept { return (Values && Ids && Indices); }

	template<typename T>
	_FORCE_INLINE_ T* GetTyped(uint16_t idx)
	{
		return (T*)Get(idx, sizeof(T));
	}
};
