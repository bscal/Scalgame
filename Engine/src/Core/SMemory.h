#pragma once

#include "Core.h"

struct UIState;

namespace Scal
{

enum MemoryTag
{
	Unknown = 0,
	Array,
	List,
	Application,
	Game,
	Resources,

	MaxTags
};

void* MemAlloc(size_t size);
void* MemAllocZero(size_t size);
void* MemRealloc(void* block, size_t size);
void MemCopy(void* dst, const void* src, size_t size);
void MemClear(void* block, size_t size);
void MemFree(void* block);

void ShowMemoryUsage(UIState* uiState);

}


