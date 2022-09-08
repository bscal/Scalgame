#include "SMemory.h"

#include <memory>

namespace Scal
{
namespace Memory
{

// TODO for now use standard malloc but should probably switch

void* Alloc(size_t size)
{
	return malloc(size);
}

void* AllocZero(size_t size)
{
	return calloc(size, size);
}

void* ReAlloc(void* block, size_t size)
{
	return realloc(block, size);
}

void Copy(void* dst, void* src, size_t size)
{
	memcpy(dst, src, size);
}

void Clear(void* block, size_t size)
{
	memset(block, 0, size);
}

void Free(void* block)
{
	free(block);
}

}
}