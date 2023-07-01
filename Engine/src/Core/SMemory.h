#pragma once

#include "Core.h"

#include <string.h> // memcpy
#include <stdlib.h> // malloc

#include <MemoryPool/MemoryPool.h>

struct GameApplication;

enum class SAllocator : uint8_t
{
	Game = 0,
	Temp,
	Malloc,

	MaxTypes,
};

enum class MemoryTag : uint8_t
{
	Unknown = 0,
	Arrays,
	Lists,
	LinkedList,
	Tables,
	Sets,
	Strings,
	Game,
	UI,
	Profiling,
	TrackedMalloc,
	Trees,

	MaxTags
};

constexpr static const char* MemoryTagStrings[(uint8_t)MemoryTag::MaxTags] =
{
	"Unknown",
	"Arrays",
	"Lists",
	"LinkedList",
	"Tables",
	"Sets",
	"Strings",
	"Game",
	"UI",
	"Profiling",
	"TrackedMalloc",
	"Trees"
};

void
SMemInitialize(GameApplication* gameApp, size_t gameMemorySize, size_t tempMemorySize);

void
SMemShutdown(GameApplication* gameApp);

void* SMemAlloc(size_t size);
void* SMemRealloc(void* block, size_t size);
void  SMemFree(void* block);

void* SMemTempAlloc(size_t size);
void  SMemTempReset();

void* SMemAllocTag(uint8_t allocator, size_t size, MemoryTag tag);
void* SMemReallocTag(uint8_t allocator, void* ptr, size_t oldSize, size_t newSize, MemoryTag tag);
void  SMemFreeTag(uint8_t allocator, void* ptr, size_t size, MemoryTag tag);

void* SMemAllocTagPrint(uint8_t allocator, size_t size, MemoryTag tag, int line, const char* file, const char* function);
void* SMemReallocTagPrint(uint8_t allocator, void* ptr, size_t oldSize, size_t newSize, MemoryTag tag, int line, const char* file, const char* function);
void  SMemFreeTagPrint(uint8_t allocator, void* ptr, size_t size, MemoryTag tag, int line, const char* file, const char* function);

_FORCE_INLINE_ void SMemCopy(void* dst, const void* src, size_t size);
_FORCE_INLINE_ void SMemMove(void* dst, const void* src, size_t size);
_FORCE_INLINE_ void SMemSet(void* block, int value, size_t size);
_FORCE_INLINE_ void SMemClear(void* block, size_t size);

bool ValidateMemory(SAllocator allocator, void* block);
bool ValidateGameMemory(void* block);
bool ValidateTempMemory(void* block);

const size_t* SMemGetTaggedUsages();
uint64_t SMemGetAllocated();
uint64_t SMemGetLastFrameTempUsage();

#define SMEM_USE_TAGS 1
#define SMEM_PRINT_ALLOCATIONS 0

#if SMEM_USE_TAGS
	#if SMEM_PRINT_ALLOCATIONS
	#define SAlloc(allocator, sz, tag) SMemAllocTagPrint((uint8_t)allocator, sz, tag, __LINE__, __FILE__, __FUNCTION__)
	#define SCalloc(allocator, n, sz, tag) SMemAllocTagPrint((uint8_t)allocator, n * sz, tag, __LINE__, __FILE__, __FUNCTION__)
	#define SRealloc(allocator, ptr, oldSz, newSz, tag) SMemReallocTagPrint((uint8_t)allocator, ptr, oldSz, newSz, tag, __LINE__, __FILE__, __FUNCTION__)
	#define SFree(allocator, ptr, sz, tag) SMemFreeTagPrint((uint8_t)allocator, ptr, sz, tag, __LINE__, __FILE__, __FUNCTION__);
	#else
	#define SAlloc(allocator, sz, tag) SMemAllocTag((uint8_t)allocator, sz, tag)
	#define SCalloc(allocator, n, sz, tag) SMemAllocTag((uint8_t)allocator, n * sz, tag)
	#define SRealloc(allocator, ptr, oldSz, newSz, tag) SMemReallocTag((uint8_t)allocator, ptr, oldSz, newSz, tag)
	#define SFree(allocator, ptr, sz, tag) SMemFreeTag((uint8_t)allocator, ptr, sz, tag);
	#endif
	#else
	#define SAlloc(allocator, sz, tag)
	#define SCalloc(allocator, n, sz, tag)
	#define SRealloc(allocator, ptr, oldSz, newSz, tag)
	#define SFree(allocator, ptr, sz, tag)
#endif

void SMemCopy(void* dst, const void* src, size_t size)
{
	SASSERT(dst);
	SASSERT(src);
	SASSERT(size > 0);
	memcpy(dst, src, size);
}

void SMemMove(void* dst, const void* src, size_t size)
{
	SASSERT(dst);
	SASSERT(src);
	SASSERT(size > 0);
	memmove(dst, src, size);
}

void SMemSet(void* dst, int value, size_t size)
{
	SASSERT(dst);
	SASSERT(size > 0);
	memset(dst, value, size);
}

void SMemClear(void* dst, size_t size)
{
	SASSERT(dst);
	SASSERT(size > 0);
	memset(dst, 0, size);
}
