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

constexpr global_var uint32_t MemoryTagUsage[MaxTags] = {};
constexpr global_var const char* MemoryTagStrings[MaxTags] =
{
	"Unknown",
	"Array",
	"List",
	"Application",
	"Game",
	"Resources",
};

void ShowMemoryUsage(UIState* uiState)
{
	RenderMemoryUsage(uiState, MaxTags, MemoryTagUsage, MemoryTagStrings);
}

}
