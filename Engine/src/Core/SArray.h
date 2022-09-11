#pragma once

#include "Core.h"

namespace Scal
{

#define ARRAY_DEFAULT_SIZE ((uint64_t)8)
#define ARRAY_DEFAULT_RESIZE ((uint64_t)2)

struct ResizableArray
{
	void* Memory;
	uint64_t Capacity;
	uint64_t Length;
	uint64_t Stride;
};

SAPI void ArrayCreate(uint64_t capacity, uint64_t stride, ResizableArray* outSArray);
SAPI void ArrayFree(ResizableArray* sArray);
SAPI void ArrayResize(ResizableArray* sArray);
SAPI uint64_t GetArrayMemorySize(ResizableArray* sArray);
SAPI void ArrayPush(ResizableArray* sArray, const void* valuePtr);
SAPI void ArrayPop(ResizableArray* sArray, void* dest);
SAPI void ArraySetAt(ResizableArray* sArray, uint64_t index, const void* valuePtr);
SAPI void ArrayPopAt(ResizableArray* sArray, uint64_t index, void* dest);
SAPI void* ArrayPeekAt(ResizableArray* sArray, uint64_t index);
SAPI void ArrayClear(ResizableArray* sArray);

#define SArrayPush(sArray, value) \
{ \
	auto tmp = value; \
	Scal::ArrayPush(sArray, &tmp); \
} \

}
