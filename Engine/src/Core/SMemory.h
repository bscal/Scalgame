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
void  SMemFreeTag(void* block, size_t size, MemoryTag tag);

void SMemCopy(void* dst, const void* src, size_t size);
void SMemMove(void* dst, const void* src, size_t size);
void SMemSet(void* block, int value, size_t size);
void SMemClear(void* block, size_t size);

const size_t* SMemGetTaggedUsages();
uint64_t SMemGetAllocated();

struct SMemAllocator
{
	void* (*Alloc)(size_t);
	void (*Free)(void*);
};

// Temp memory gets freed at start of a frame
static void SMemTempAllocatorFree(void* block) {};

static const SMemAllocator SMEM_GAME_ALLOCATOR = { SMemAlloc, SMemFree };
static const SMemAllocator SMEM_TEMP_ALLOCATOR = { SMemTempAlloc, SMemTempAllocatorFree };

inline bool IsTemporaryAllocator(const SMemAllocator* allocator)
{
	return allocator->Alloc == SMemTempAlloc;
};
