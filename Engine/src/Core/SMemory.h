#pragma once

#include "rmem/rmem.h"

#include <stdint.h>

struct GameApplication;
struct UIState;

enum class MemoryTag : uint8_t
{
	Unknown = 0,
	Arrays,
	Lists,
	Tables,
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

void SMemCopyAligned(void* dst, const void* src, size_t size, size_t alignment);
void SMemSetAligned(void* block, int value, size_t size, size_t alignment);
void SMemClearAligned(void* block, size_t size, size_t alignment);

int GetNewCalls();

const uint64_t* SMemGetTaggedUsages();
uint64_t SMemGetUsage();
uint64_t SMemGetAllocated();

MemPool* const GetGameMemory();
BiStack* const GetTempMemory();
