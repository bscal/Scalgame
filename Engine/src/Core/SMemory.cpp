#include "SMemory.h"

#include <memory>
#include "SUI.h"

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


constexpr global_var const char* MemoryTagStrings[MaxTags] =
{
	"Unknown",
	"Array",
	"List",
	"Application",
	"Game",
	"Resources",
};

global_var uint32_t MemoryTagUsage[MaxTags] = {};

void* MemAllocTag(size_t size, MemoryTag tag)
{
	if (tag == MemoryTag::Unknown)
	{
		TraceLog(LOG_ERROR, "Cannot allocate memory with Unknown tag");
		return nullptr;
	}

	MemoryTagUsage[tag] += (uint32_t)size;
	return MemAlloc(size);
}

void MemFreeTag(void* block, size_t size, MemoryTag tag)
{
	if (tag == MemoryTag::Unknown)
	{
		TraceLog(LOG_ERROR, "Cannot free memory with Unknown tag");
		return;
	}

	MemoryTagUsage[tag] -= (uint32_t)size;
	Scal::MemFree(block);
}

void ShowMemoryUsage(UIState* uiState)
{
	RenderMemoryUsage(uiState, MaxTags, MemoryTagUsage, MemoryTagStrings);
}

}
