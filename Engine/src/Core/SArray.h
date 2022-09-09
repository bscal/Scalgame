#pragma once

#include "Core.h"

namespace Scal
{
namespace Array
{
struct SArray
{
	void* Memory;
	uint64_t Capacity;
	uint64_t Length;
	uint64_t Stride;
};

struct SArrayIterator
{
	SArray* Array;
	uint64_t Index;
	uint64_t Offset;

	bool HasNext() const;
	void* Next();
	void* Peek();
	void Remove();
};

SAPI void ArrayCreate(uint64_t capacity, uint64_t stride, SArray* outSArray);
SAPI void ArrayDestroy(SArray* sArray);
SAPI SArray* ArrayResize(SArray* sArray);
SAPI uint64_t GetArrayMemorySize(SArray* sArray);
SAPI void ArrayPush(SArray* sArray, const void* valuePtr);
SAPI void ArrayPop(SArray* sArray, void* dest);
SAPI void ArraySetAt(SArray* sArray, uint64_t index, const void* valuePtr);
SAPI void ArrayPopAt(SArray* sArray, uint64_t index, void* dest);
SAPI void* ArrayPeekAt(SArray* sArray, uint64_t index);
SAPI void ArrayClear(SArray* sArray);

#define SArrayPush(sArray, value) \
{ \
	auto tmp = value; \
	ArrayPush(sArray, &tmp); \
} \

}
}
