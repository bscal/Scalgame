#pragma once

#include "Core.h"
#include "rmem/rmem.h"

struct GameApplication;
struct UIState;

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

constexpr static const char* MemoryTagStrings[(uint8_t)MemoryTag::MaxTags] =
{
	"Unknown",
	"Arrays",
	"Lists",
	"Tables",
	"Strings",
	"Game",
	"UI"
};

void
SMemInitialize(GameApplication* gameApp,
	uint64_t gameMemorySize, uint64_t tempMemorySize);

void* SMemAlloc(size_t size);
void* SMemRealloc(void* block, size_t size);
void  SMemFree(void* block);

void* SMemTempAlloc(size_t size);
void SMemTempReset();

void* SMemStdAlloc(size_t size);
void* SMemStdRealloc(void* block, size_t size);
void  SMemStdFree(void* block);

void* SMemAllocTag(size_t size, MemoryTag tag);
void* SMemReallocTag(void* block, size_t oldSize, size_t newSize, MemoryTag tag);
void  SMemFreeTag(void* block, size_t size, MemoryTag tag);

void SMemCopy(void* dst, const void* src, size_t size);
void SMemMove(void* dst, const void* src, size_t size);
void SMemSet(void* block, int value, size_t size);
void SMemClear(void* block, size_t size);

int GetNewCalls();

const uint64_t* SMemGetTaggedUsages();
uint64_t SMemGetUsage();
uint64_t SMemGetAllocated();

MemPool* const GetGameMemory();
BiStack* const GetTempMemory();

struct SMemAllocator
{
	void* (*Alloc)(size_t);
	void (*Free)(void*);
};

// Temp memory gets freed at start of a frame
internal inline void
SMemTempAllocatorFree(void* block)
{
};

global_var const SMemAllocator SMEM_GAME_ALLOCATOR = { SMemAlloc, SMemFree };
global_var const SMemAllocator SMEM_TEMP_ALLOCATOR = { SMemTempAlloc, SMemTempAllocatorFree };

inline bool IsTemporaryAllocator(const SMemAllocator* allocator)
{
	SASSERT(allocator);
	return allocator->Alloc == SMemTempAlloc;
};

struct SMemAllocO
{
	[[nodiscard]] inline void* operator()(size_t n, MemoryTag tag) const noexcept
	{
		void* mem = SMemAllocTag(n, tag);
		SASSERT(mem);
		return mem;
	}
};

struct SMemFreeO
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
		return AllocatorFunc(size, tag);
	}

	inline void Free(void* block, size_t size, MemoryTag tag) const noexcept
	{
		FreeFunc(block, size, tag);
	}
};

global_var constexpr const SMemTagAllocator<SMemAllocO, SMemFreeO> SMemTagGameAllocator;
