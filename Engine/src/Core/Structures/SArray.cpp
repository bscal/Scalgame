#include "SArray.h"

#include "Core/SMemory.h"

internal inline bool 
CheckBounds(SArray* sArray, uint32_t index)
{
	SASSERT(sArray);
	SASSERT(sArray->Memory);
	SASSERT(sArray->Stride > 0);
	uint8_t* dst = (uint8_t*)sArray->Memory + (index * sArray->Stride);
	uint8_t* end = (uint8_t*)sArray->Memory + (sArray->Capacity * sArray->Stride);

	SASSERT(dst >= sArray->Memory);

	if (sArray->Capacity > 0)
	{
		SASSERT(sArray->Memory < end);
		SASSERT(dst < end);
	}
	else
	{
		SASSERT(sArray->Memory == end);
		SASSERT(dst == sArray->Memory);
	}
	return true;
}



SAPI SArray ArrayCreate(uint8_t allocator, uint32_t capacity, uint32_t stride)
{
	SASSERT(allocator < (uint8_t)SAllocator::MaxTypes);
	SASSERT(stride > 0);
	SArray sArray;
	sArray.Memory = nullptr;
	sArray.Capacity = 0;
	sArray.Count = 0;
	sArray.Stride = stride;
	sArray.Allocator = allocator;
	if (capacity > 0)
		ArrayResize(&sArray, capacity);
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
	size_t size = (size_t)sArray->Capacity * sArray->Stride;
	SMemFreeTag(sArray->Allocator, sArray->Memory, size, MemoryTag::Arrays);
}

SAPI void ArrayResize(SArray* sArray, uint32_t newCapacity)
{
	SASSERT(sArray);
	SASSERT(sArray->Stride > 0);

	if (newCapacity == 0)
		newCapacity = SARRAY_DEFAULT_SIZE;

	if (newCapacity <= sArray->Capacity)
		return;

	size_t oldSize = (size_t)sArray->Capacity * sArray->Stride;
	size_t newSize = (size_t)newCapacity * sArray->Stride;
	sArray->Memory = SMemReallocTag(sArray->Allocator, sArray->Memory, oldSize, newSize, MemoryTag::Arrays);
	sArray->Capacity = newCapacity;
}

SAPI void ArraySetCount(SArray* sArray, uint32_t count)
{
	SASSERT(sArray);

	uint32_t initialCount = sArray->Count;
	if (initialCount >= count)
		return;

	ArrayResize(sArray, count);

	size_t start = (size_t)initialCount * sArray->Stride;
	size_t end = (size_t)sArray->Capacity * sArray->Stride;
	SASSERT(end - start > 0);
	SASSERT((uint8_t*)sArray->Memory + start <= (uint8_t*)sArray->Memory + end);
	SMemClear((uint8_t*)sArray->Memory + start, end - start);
	sArray->Count = count;
}

SAPI size_t GetArrayMemorySize(SArray* sArray)
{
	SASSERT(sArray);
	return sArray->Capacity * sArray->Stride;
}

SAPI void ArrayPush(SArray* sArray, const void* valuePtr)
{
	SASSERT(sArray);

	if (sArray->Count == sArray->Capacity)
	{
		ArrayResize(sArray, sArray->Capacity * SARRAY_DEFAULT_RESIZE);
	}

	char* dest = (char*)sArray->Memory;
	size_t offset = (size_t)sArray->Count * sArray->Stride;
	SMemCopy(dest + offset, valuePtr, sArray->Stride);
	++sArray->Count;
}

SAPI void ArrayPop(SArray* sArray, void* dest)
{
	SASSERT(sArray);

	if (sArray->Count == 0) return;

	const char* src = (char*)(sArray->Memory);
	size_t offset = (size_t)(sArray->Count - 1) * sArray->Stride;
	SMemCopy(dest, src + offset, sArray->Stride);
	--sArray->Count;
}

SAPI void ArraySetAt(SArray* sArray, uint32_t index, const void* valuePtr)
{
	SASSERT(CheckBounds(sArray, index));

	char* dest = (char*)(sArray->Memory);
	size_t offset = index * sArray->Stride;
	SMemCopy(dest + offset, valuePtr, sArray->Stride);
}

SAPI void ArrayPopAt(SArray* sArray, uint32_t index, void* dest)
{
	SASSERT(CheckBounds(sArray, index));

	size_t offset = (size_t)index * sArray->Stride;
	char* popAtAddress = (char*)(sArray->Memory) + offset;
	SMemCopy(dest, popAtAddress, sArray->Stride);
	if (index != sArray->Count)
	{
		// Moves last element in array popped position
		size_t lastIndexOffset = (size_t)sArray->Count * sArray->Stride;
		char* lastIndexAddress = (char*)(sArray->Memory) + lastIndexOffset;
		SMemCopy(popAtAddress, lastIndexAddress, sArray->Stride);
	}
	--sArray->Count;
}

SAPI void* ArrayPeekAt(SArray* sArray, uint32_t index)
{
	SASSERT(CheckBounds(sArray, index));

	size_t offset = (size_t)index * sArray->Stride;
	return ((char*)(sArray->Memory) + offset);
}

SAPI void ArrayClear(SArray* sArray)
{
	SASSERT(sArray);
	sArray->Count = 0;
}

SAPI void ArrayRemove(SArray* sArray)
{
	SASSERT(sArray);
	if (sArray->Count > 0)
	{
		--sArray->Count;
	}
}

SAPI bool ArrayRemoveAt(SArray* sArray, uint32_t index)
{
	SASSERT(CheckBounds(sArray, index));

	size_t offset = (size_t)index * sArray->Stride;
	char* address = ((char*)(sArray->Memory)) + offset;
	bool shouldMoveLast = sArray->Count != 0 && index != (sArray->Count - 1);
	if (shouldMoveLast)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = (size_t)(sArray->Count - 1) * sArray->Stride;
		char* lastIndexAddress = ((char*)(sArray->Memory)) + lastIndexOffset;
		SMemCopy(address, lastIndexAddress, sArray->Stride);
	}
	--sArray->Count;
	return shouldMoveLast;
}

