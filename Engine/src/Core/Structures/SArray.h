#pragma once

#include "Core/Core.h"

#define SARRAY_DEFAULT_SIZE ((uint64_t)8)
#define SARRAY_DEFAULT_RESIZE ((uint64_t)2)

struct SArray
{
	void* Memory;
	uint64_t Capacity;
	uint64_t Length;
	uint64_t Stride;
};

SAPI void ArrayCreate(uint64_t capacity, uint64_t stride, SArray* outSArray);
SAPI void ArrayFree(SArray* sArray);
SAPI void ArrayResize(SArray* sArray);
SAPI uint64_t GetArrayMemorySize(SArray* sArray);
SAPI void ArrayPush(SArray* sArray, const void* valuePtr);
SAPI void ArrayPop(SArray* sArray, void* dest);
SAPI void ArraySetAt(SArray* sArray, uint64_t index, const void* valuePtr);
SAPI void ArrayPopAt(SArray* sArray, uint64_t index, void* dest);
SAPI void* ArrayPeekAt(SArray* sArray, uint64_t index);
SAPI void ArrayClear(SArray* sArray);

#define SArrayCreate(stride, outSArray) ArrayCreate(SARRAY_DEFAULT_SIZE, stride, outSArray)
#define SArrayPush(sArray, value) \
{ \
	auto tmp = value; \
	Scal::ArrayPush(sArray, &tmp); \
} \