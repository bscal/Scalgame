#pragma once

#include <stdint.h>

struct GameApplication;

namespace SAllocator
{
enum Type
{
	Invalid = -1,
	Game = 0,
	Temp,

	MaxTypes
};
}

enum class MemoryTag : uint8_t
{
	Unknown = 0,
	Not_Tracked,
	Arrays,
	Lists,
	Tables,
	Strings,
	Game,
	UI,
	Profiling,

	MaxTags
};

static constexpr const char* MemoryTagStrings[(uint8_t)MemoryTag::MaxTags] =
{
	"Unknown",
	"Not Tracked",
	"Arrays",
	"Lists",
	"Tables",
	"Strings",
	"Game",
	"UI",
	"Profiling"
};

void
SMemInitialize(GameApplication* gameApp, size_t gameMemorySize, size_t tempMemorySize);

void SMemFree();


void* SMemAlloc(size_t size);
void* SMemRealloc(void* block, size_t size);
void  SMemFree(void* block);

void* SMemTempAlloc(size_t size);
void  SMemTempReset();

void* SMemAllocTag(int allocator, size_t size, MemoryTag tag);
void* SMemReallocTag(int allocator, void* ptr, size_t oldSize, size_t newSize, MemoryTag tag);
void  SMemFreeTag(int allocator, void* ptr, size_t size, MemoryTag tag);

void SMemCopy(void* dst, const void* src, size_t size);
void SMemMove(void* dst, const void* src, size_t size);
void SMemSet(void* block, int value, size_t size);
void SMemClear(void* block, size_t size);

const size_t* SMemGetTaggedUsages();
uint64_t SMemGetAllocated();

#define SMEM_USE_TAGS 1
#if SMEM_USE_TAGS
#define SAlloc(allocator, sz, tag) SMemAllocTag(allocator, sz, tag)
#define SCalloc(allocator, n, sz, tag) SMemAllocTag(allocator, n * sz, tag)
#define SRealloc(allocator, ptr, oldSz, newSz, tag) SMemReallocTag(allocator, ptr, oldSz, newSz, tag)
#define SFree(allocator, ptr, sz, tag) SMemFreeTag(allocator, ptr, sz, tag);
#else
#define SAlloc(allocator, sz, tag)
#define SCalloc(allocator, n, sz, tag)
#define SRealloc(allocator, ptr, oldSz, newSz, tag)
#define SFree(allocator, ptr, sz, tag)
#endif
