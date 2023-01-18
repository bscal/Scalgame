#include "SArray.h"

#include "Core/SMemory.h"

SAPI SArray ArrayCreate(uint64_t capacity, uint64_t stride)
{
	SASSERT(capacity > 0);
	SASSERT(stride > 0);

	SArray sArray;
	sArray.Memory = (SArray*)SMemAlloc(capacity * stride);
	sArray.Capacity = capacity;
	sArray.Count = 0;
	sArray.Stride = stride;
	return sArray;
}

SAPI void ArrayFree(SArray* sArray)
{
	SASSERT(sArray);
	if (!sArray->Memory)
	{
		TraceLog(LOG_ERROR, "outSArray Memory is already freed!");
		return;
	}
	SMemFree(sArray->Memory);
}

SAPI void ArrayResize(SArray* sArray)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);
	sArray->Capacity *= SARRAY_DEFAULT_RESIZE;
	uint64_t newSize = sArray->Capacity * sArray->Stride;
	sArray->Memory = SMemRealloc(sArray->Memory, newSize);
}

SAPI uint64_t GetArrayMemorySize(SArray* sArray)
{
	SASSERT(sArray);
	return sArray->Capacity * sArray->Stride;
}

SAPI void ArrayPush(SArray* sArray, const void* valuePtr)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);

	if (sArray->Count == sArray->Capacity)
	{
		ArrayResize(sArray);
	}

	uint64_t offset = sArray->Count * sArray->Stride;
	char* dest = (char*)sArray->Memory;
	SMemCopy(dest + offset, valuePtr, sArray->Stride);
	++sArray->Count;
}

SAPI void ArrayPop(SArray* sArray, void* dest)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);

	if (sArray->Count == 0) return;
	uint64_t offset = (sArray->Count - 1) * sArray->Stride;
	const char* src = (char*)(sArray->Memory);
	SMemCopy(dest, src + offset, sArray->Stride);
	--sArray->Count;
}

SAPI void ArraySetAt(SArray* sArray, uint64_t index, const void* valuePtr)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);
	SASSERT(index <= sArray->Count);

	uint64_t offset = index * sArray->Stride;
	char* dest = (char*)(sArray->Memory) + offset;
	SMemCopy(dest, valuePtr, sArray->Stride);
}

SAPI void ArrayPopAt(SArray* sArray, uint64_t index, void* dest)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);
	SASSERT(index <= sArray->Count);

	uint64_t offset = index * sArray->Stride;
	char* popAtAddress = (char*)(sArray->Memory) + offset;
	SMemCopy(dest, popAtAddress, sArray->Stride);
	if (index != sArray->Count)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = sArray->Count * sArray->Stride;
		char* lastIndexAddress = (char*)(sArray->Memory) + lastIndexOffset;
		SMemCopy(popAtAddress, lastIndexAddress, sArray->Stride);
	}
	--sArray->Count;
}

SAPI void* ArrayPeekAt(SArray* sArray, uint64_t index)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);
	SASSERT(index <= sArray->Count);

	uint64_t offset = index * sArray->Stride;
	return (char*)(sArray->Memory) + offset;
}

SAPI void ArrayClear(SArray* sArray)
{
	SASSERT(sArray);
	sArray->Count = 0;
}

SAPI bool ArrayRemoveAt(SArray* sArray, uint64_t index)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);
	SASSERT(index <= sArray->Count);

	size_t offset = index * sArray->Stride;
	char* address = ((char*)(sArray->Memory)) + offset;
	bool shouldMoveLast = sArray->Count != 0 && index != (sArray->Count - 1);
	if (shouldMoveLast)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = (sArray->Count - 1) * sArray->Stride;
		char* lastIndexAddress = ((char*)(sArray->Memory)) + lastIndexOffset;
		SMemCopy(address, lastIndexAddress, sArray->Stride);
	}
	--sArray->Count;
	return shouldMoveLast;
}

