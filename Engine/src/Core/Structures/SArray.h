#pragma once

#include "Core/Core.h"

#define SARRAY_DEFAULT_SIZE (1u)
#define SARRAY_DEFAULT_RESIZE (2u)

struct SArray
{
	void* Memory;
	uint32_t Capacity;
	uint32_t Count;
	uint32_t Stride;
	uint8_t Allocator;
};

SAPI SArray ArrayCreate(uint8_t allocator, uint32_t capacity, uint32_t stride);
SAPI void ArrayFree(SArray* sArray);
SAPI void ArrayResize(SArray* sArray, uint32_t newCapacity);
SAPI void ArraySetCount(SArray* sArray, uint32_t count);
SAPI uint64_t GetArrayMemorySize(SArray* sArray);
SAPI void ArrayPush(SArray* sArray, const void* valuePtr);
SAPI void ArrayPop(SArray* sArray, void* dest);
SAPI void ArraySetAt(SArray* sArray, uint32_t index, const void* valuePtr);
SAPI void ArrayPopAt(SArray* sArray, uint32_t index, void* dest);
SAPI void* ArrayPeekAt(SArray* sArray, uint32_t index);
SAPI void ArrayClear(SArray* sArray);
SAPI void ArrayRemove(SArray* sArray);
SAPI bool ArrayRemoveAt(SArray* sArray, uint32_t index);

#define ArrayEmptyTyped(T, allocator) (SArray{ nullptr, 0, 0, sizeof(T), (uint8_t)allocator })
