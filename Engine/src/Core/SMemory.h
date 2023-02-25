#pragma once

#include "rmem/rmem.h"

#include <stdint.h>

struct GameApplication;

enum class MemoryTag : uint8_t
{
	Unknown = 0,
	Arrays,
	Lists,
	Tables,
	Strings,
	Game,
	UI,

	MaxTags
};

static constexpr const char* MemoryTagStrings[(uint8_t)MemoryTag::MaxTags] =
{
	"Unknown",
	"Arrays",
	"Lists",
	"Tables",
	"Strings",
	"Game",
	"UI",
};

void
SMemInitialize(GameApplication* gameApp,
	uint64_t gameMemorySize, uint64_t tempMemorySize);

void* SMemAlloc(size_t size);
void* SMemRealloc(void* block, size_t size);
void  SMemFree(void* block);

void* SMemTempAlloc(size_t size);
void  SMemTempReset();

void* SMemAllocTag(size_t size, MemoryTag tag);
void* SMemReallocTag(void* block, size_t oldSize, size_t newSize, MemoryTag tag);
void  SMemFreeTag(void* block, size_t size, MemoryTag tag);

void SMemCopy(void* dst, const void* src, size_t size);
void SMemMove(void* dst, const void* src, size_t size);
void SMemSet(void* block, int value, size_t size);
void SMemClear(void* block, size_t size);

const uint64_t* SMemGetTaggedUsages();
uint64_t SMemGetUsage();
uint64_t SMemGetAllocated();

// Overrides malloc calls for raylib
#ifndef RL_MALLOC
#define RL_MALLOC(sz) SMemAlloc(sz)
#endif
#ifndef RL_CALLOC
#define RL_CALLOC(n,sz) SMemAlloc(n * sz)
#endif
#ifndef RL_REALLOC
#define RL_REALLOC(ptr,sz) SMemRealloc(ptr, 0, sz)
#endif
#ifndef RL_FREE
#define RL_FREE(ptr) SMemFree(ptr)
#endif

struct SMemAllocator
{
	void* (*Alloc)(size_t);
	void (*Free)(void*);
};

// Temp memory gets freed at start of a frame
static void
SMemTempAllocatorFree(void* block)
{
};

static const SMemAllocator SMEM_GAME_ALLOCATOR = { SMemAlloc, SMemFree };
static const SMemAllocator SMEM_TEMP_ALLOCATOR = { SMemTempAlloc, SMemTempAllocatorFree };

inline bool IsTemporaryAllocator(const SMemAllocator* allocator)
{
	return allocator->Alloc == SMemTempAlloc;
};

struct SMemAllocatorFunc
{
	[[nodiscard]] inline void* operator()(size_t n, MemoryTag tag) const noexcept
	{
		void* mem = SMemAllocTag(n, tag);
		return mem;
	}
};

struct SMemFreeFunc
{
	inline void Free(void* block, size_t n, MemoryTag tag) const noexcept
	{
		SMemFreeTag(block, n, tag);
	}
};

template<typename AllocatorFunc, typename FreeFunc>
struct SMemTagAllocator
{
	[[nodiscard]] inline void* Alloc(size_t size, MemoryTag tag) const noexcept
	{ 
		return AllocatorFunc{}(size, tag);
	}

	inline void Free(void* block, size_t size, MemoryTag tag) const noexcept
	{
		FreeFunc{}(block, size, tag);
	}
};

static const SMemTagAllocator<SMemAllocatorFunc, SMemFreeFunc> SMemTagGameAllocator;
