#pragma once

#include "rmem/rmem.h"

#include <stdint.h>

struct UIState;

namespace Scal
{

enum MemoryTag
{
	Unknown = 0,
	Arrays,
	Lists,
	Tables,
	Sets,
	Pools,
	Game,
	GameMemory,
	UI,

	MaxTags
};

constexpr static const char* MemoryTagStrings[MaxTags] =
{
	"Unknown",
	"Arrays",
	"Lists",
	"Tables",
	"Sets",
	"Pools",
	"Game",
	"GameMemory",
	"UI"
};

void* MemAlloc(size_t size);
void* MemAllocZero(size_t size);
void* MemRealloc(void* block, size_t size);
void MemFree(void* block);

void* MemAllocTag(size_t size, MemoryTag tag);
void* MemReallocTag(void* block, size_t oldSize, size_t newSize, MemoryTag tag);
void  MemFreeTag(void* block, size_t size, MemoryTag tag);

void MemCopy(void* dst, const void* src, size_t size);
void MemSet(void* block, int value, size_t size);
void MemClear(void* block, size_t size);

const size_t* GetMemoryUsage();

}
