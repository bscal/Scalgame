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
uint64_t SMemGetTempAllocated();

MemPool* const GetGameMemory();
BiStack* const GetTempMemory();

internal inline void* 
SMemAllactorAlloc(size_t size)
{
	return SMemAlloc(size);
}

internal inline void 
SMemAllactorFree(void* block)
{
	return SMemFree(block);
}

struct SMemAllocator
{
	void* (*Alloc)(size_t) = SMemAllactorAlloc;
	void (*Free)(void*) = SMemAllactorFree;
};

struct SMemTempAllocator : public SMemAllocator
{
	[[nodiscard]] static void* SMemTempAllocatorAlloc(size_t size)
	{
		void* allocation = BiStackAllocFront(GetTempMemory(), size);
		SASSERT(allocation);
		return allocation;
	}

	static void SMemTempAllocatorFree(void* block) {};

	SMemTempAllocator()
	{
		Alloc = SMemTempAllocatorAlloc;
		Free = SMemTempAllocatorFree;
	}
};