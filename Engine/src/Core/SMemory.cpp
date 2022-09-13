#include "SMemory.h"

#include <memory>

namespace Scal
{
// TODO for now use standard malloc but should probably switch

void* MemAlloc(size_t size)
{
	return malloc(size);
}

void* MemAllocZero(size_t size)
{
	return calloc(size, size);
}

void* MemRealloc(void* block, size_t size)
{
	return realloc(block, size);
}

void MemCopy(void* dst, const void* src, size_t size)
{
	memcpy(dst, src, size);
}

void MemClear(void* block, size_t size)
{
	memset(block, 0, size);
}

void MemFree(void* block)
{
	free(block);
}

}