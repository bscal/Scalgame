#include "SArray.h"

#include "SMemory.h"

#include <cassert>

namespace Scal
{

SAPI void ArrayCreate(uint64_t capacity, uint64_t stride, ResizableArray* outSArray)
{
	assert(stride > 0, "capacity must be > 0");
	assert(outSArray != nullptr, "outSArray cannot be nullptr");

	if (capacity == 0)
	{
		TraceLog(LOG_WARNING, "capacity is equal to 0, setting to DEFAULT_SIZE");
		capacity = ARRAY_DEFAULT_SIZE;
	}
	if (outSArray->Memory)
	{
		TraceLog(LOG_ERROR, "outSArray Memory is already allocated!");
		return;
	}

	ResizableArray sArray = {};
	sArray.Memory = (ResizableArray*)Memory::Alloc(capacity * stride);
	sArray.Capacity = capacity;
	sArray.Stride = stride;
	*outSArray = sArray;
}

SAPI void ArrayFree(ResizableArray* sArray)
{
	assert(sArray);
	if (!sArray->Memory)
	{
		TraceLog(LOG_ERROR, "outSArray Memory is already freed!");
		return;
	}
	Memory::Free(sArray->Memory);
}

SAPI void ArrayResize(ResizableArray* sArray)
{
	assert(sArray);
	assert(sArray->Memory);
	sArray->Capacity *= ARRAY_DEFAULT_RESIZE;
	uint64_t newSize = sArray->Capacity * sArray->Stride;
	sArray->Memory = Memory::ReAlloc(sArray->Memory, newSize);
}

SAPI uint64_t GetArrayMemorySize(ResizableArray* sArray)
{
	assert(sArray);
	return sArray->Capacity * sArray->Stride;
}

SAPI void ArrayPush(ResizableArray* sArray, const void* valuePtr)
{
	assert(sArray);
	assert(sArray->Memory);

	if (sArray->Length >= sArray->Capacity)
	{
		ArrayResize(sArray);
	}
	uint64_t offset = sArray->Length * sArray->Stride;
	char* dest = (char*)sArray->Memory;
	Memory::Copy(dest + offset, valuePtr, sArray->Stride);
	++sArray->Length;
}

SAPI void ArrayPop(ResizableArray* sArray, void* dest)
{
	assert(sArray);
	assert(sArray->Memory);

	if (sArray->Length == 0) return;
	uint64_t offset = (sArray->Length - 1) * sArray->Stride;
	const char* src = (char*)(sArray->Memory);
	Memory::Copy(dest, src + offset, sArray->Stride);
	--sArray->Length;
}

SAPI void ArraySetAt(ResizableArray* sArray, uint64_t index, const void* valuePtr)
{
	assert(sArray);
	assert(sArray->Memory);
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	char* dest = (char*)(sArray->Memory) + offset;
	Memory::Copy(dest, valuePtr, sArray->Stride);
}

SAPI void ArrayPopAt(ResizableArray* sArray, uint64_t index, void* dest)
{
	assert(sArray);
	assert(sArray->Memory);
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	char* popAtAddress = (char*)(sArray->Memory) + offset;
	Memory::Copy(dest, popAtAddress, sArray->Stride);
	if (index != sArray->Length)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = sArray->Length * sArray->Stride;
		char* lastIndexAddress = (char*)(sArray->Memory) + lastIndexOffset;
		Memory::Copy(popAtAddress, lastIndexAddress, sArray->Stride);
	}
	--sArray->Length;
}

SAPI void* ArrayPeekAt(ResizableArray* sArray, uint64_t index)
{
	assert(sArray);
	assert(sArray->Memory);
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	return (char*)(sArray->Memory) + offset;
}

SAPI void ArrayClear(ResizableArray* sArray)
{
	assert(sArray);
	sArray->Length = 0;
}

}