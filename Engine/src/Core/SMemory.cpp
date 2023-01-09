#define RMEM_IMPLEMENTATION
#include "SMemory.h"


#include "Core.h"
#include "SUI.h"

#include <stdlib.h>
#include <memory>

#include <string>
#include <vector>

namespace Scal
{
// TODO for now use standard malloc but should probably switch

void* MemAlloc(size_t size)
{
	return malloc(size);
}

void* MemAllocZero(size_t size)
{
	void* memory = MemAlloc(size);
	MemSet(memory, 0, size);
	return memory;
}

void* MemRealloc(void* block, size_t size)
{
	void* mem = realloc(block, size);
	SASSERT(mem);
	return mem;
}

void MemFree(void* block)
{
	free(block);
}

void MemCopy(void* dst, const void* src, size_t size)
{
	memcpy(dst, src, size);
}

void MemSet(void* block, int value, size_t size)
{
	memset(block, value, size);
}

void MemClear(void* block, size_t size)
{
	memset(block, 0, size);
}

global_var size_t MemoryTagUsage[MaxTags];
global_var size_t TotalUsage;

void* MemAllocTag(size_t size, MemoryTag tag)
{
	SASSERT(tag != MemoryTag::Unknown);

	MemoryTagUsage[tag] += size;
	TotalUsage += size;
	return Scal::MemAlloc(size);
}

void* MemReallocTag(void* block, size_t oldSize, size_t newSize, MemoryTag tag)
{
	SASSERT(tag != MemoryTag::Unknown);

	MemoryTagUsage[tag] -= oldSize;
	MemoryTagUsage[tag] += newSize;
	TotalUsage -= oldSize;
	TotalUsage += newSize;
	return Scal::MemRealloc(block, newSize);
}

void MemFreeTag(void* block, size_t size, MemoryTag tag)
{
	SASSERT(block);
	SASSERT(tag != MemoryTag::Unknown);

	MemoryTagUsage[tag] -= size;
	TotalUsage -= size;
	Scal::MemFree(block);
}

const size_t* GetMemoryUsage()
{
	return MemoryTagUsage;
}

size_t GetTotalUsage()
{
	return TotalUsage;
}

}

global_var uint32_t NewUsageCount;

void* operator new(size_t size)
{
	++NewUsageCount;
	return malloc(size);
}

void operator delete(void* mem)
{
	free(mem);
}

void* operator new[](size_t size)
{
	++NewUsageCount;
	return malloc(size);
}

void operator delete[](void* mem)
{
	free(mem);
}

uint32_t GetNewCalls()
{
	return NewUsageCount;
}
