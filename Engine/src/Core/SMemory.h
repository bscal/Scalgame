#pragma once

#include "Core.h"
#include "rmem/rmem.h"

struct GameApplication;
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
	New,
	NewArray,

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
	"UI",
	"New",
	"NewArray"
};

void 
SMemInitialize(GameApplication* gameApp,
	uint64_t PermenantMemSize, uint64_t GameMemSize);

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
size_t GetTotalUsage();

}

uint32_t GetNewCalls();

//#if SCAL_DEBUG
//inline void* operator new(size_t size, const char* file, int line)
//{
//	SLOG_DEBUG("New called: %s, %d", file, line);
//	return Scal::MemAlloc(size);
//}
//#endif
//
//#ifdef SCAL_DEBUG
//#define SCAL_DEBUG_NEW new(__FILE__, __LINE__);
//#else
//#define SCAL_DEBUG_NEW new
//#endif
//#define new new(__FILE__, __LINE__)