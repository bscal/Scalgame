#include "SArray.h"

#include "Core/SMemory.h"

#include <assert.h>

SAPI void ArrayCreate(uint64_t capacity, uint64_t stride, SArray* outSArray)
{
	assert(stride > 0);
	assert(outSArray != nullptr);

	if (capacity == 0)
	{
		TraceLog(LOG_WARNING, "capacity is equal to 0, setting to DEFAULT_SIZE");
		capacity = SARRAY_DEFAULT_SIZE;
	}
	if (outSArray->Memory)
	{
		TraceLog(LOG_ERROR, "outSArray Memory is already allocated!");
		return;
	}

	SArray sArray = {};
	sArray.Memory = (SArray*)Scal::MemAlloc(capacity * stride);
	sArray.Capacity = capacity;
	sArray.Stride = stride;
	*outSArray = sArray;
}

SAPI void ArrayFree(SArray* sArray)
{
	assert(sArray);
	if (!sArray->Memory)
	{
		TraceLog(LOG_ERROR, "outSArray Memory is already freed!");
		return;
	}
	Scal::MemFree(sArray->Memory);
}

SAPI void ArrayResize(SArray* sArray)
{
	assert(sArray);
	assert(sArray->Memory);
	sArray->Capacity *= SARRAY_DEFAULT_RESIZE;
	uint64_t newSize = sArray->Capacity * sArray->Stride;
	sArray->Memory = Scal::MemRealloc(sArray->Memory, newSize);
}

SAPI uint64_t GetArrayMemorySize(SArray* sArray)
{
	assert(sArray);
	return sArray->Capacity * sArray->Stride;
}

SAPI void ArrayPush(SArray* sArray, const void* valuePtr)
{
	assert(sArray);
	assert(sArray->Memory);

	if (sArray->Length >= sArray->Capacity)
	{
		ArrayResize(sArray);
	}
	uint64_t offset = sArray->Length * sArray->Stride;
	char* dest = (char*)sArray->Memory;
	Scal::MemCopy(dest + offset, valuePtr, sArray->Stride);
	++sArray->Length;
}

SAPI void ArrayPop(SArray* sArray, void* dest)
{
	assert(sArray);
	assert(sArray->Memory);

	if (sArray->Length == 0) return;
	uint64_t offset = (sArray->Length - 1) * sArray->Stride;
	const char* src = (char*)(sArray->Memory);
	Scal::MemCopy(dest, src + offset, sArray->Stride);
	--sArray->Length;
}

SAPI void ArraySetAt(SArray* sArray, uint64_t index, const void* valuePtr)
{
	assert(sArray);
	assert(sArray->Memory);
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	char* dest = (char*)(sArray->Memory) + offset;
	Scal::MemCopy(dest, valuePtr, sArray->Stride);
}

SAPI void ArrayPopAt(SArray* sArray, uint64_t index, void* dest)
{
	assert(sArray);
	assert(sArray->Memory);
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	char* popAtAddress = (char*)(sArray->Memory) + offset;
	Scal::MemCopy(dest, popAtAddress, sArray->Stride);
	if (index != sArray->Length)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = sArray->Length * sArray->Stride;
		char* lastIndexAddress = (char*)(sArray->Memory) + lastIndexOffset;
		Scal::MemCopy(popAtAddress, lastIndexAddress, sArray->Stride);
	}
	--sArray->Length;
}

SAPI void* ArrayPeekAt(SArray* sArray, uint64_t index)
{
	assert(sArray);
	assert(sArray->Memory);
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	return (char*)(sArray->Memory) + offset;
}

SAPI void ArrayClear(SArray* sArray)
{
	assert(sArray);
	sArray->Length = 0;
}
