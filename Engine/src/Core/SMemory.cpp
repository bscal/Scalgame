#define RMEM_IMPLEMENTATION
#include "SMemory.h"

#include "Game.h"

#include <stdlib.h>

global_var struct MemPool* GameMemory;
global_var struct BiStack* TempMemory;
global_var uint64_t MemoryTagUsage[(uint8_t)MemoryTag::MaxTags];
global_var uint64_t TotalUsage;
global_var uint64_t GameMemSize;
global_var uint64_t TemporaryMemSize;
global_var uint64_t TotalMemoryAllocated;

internal inline size_t AlignSize(size_t size, size_t alignment)
{
    return (size + (alignment - 1)) & -alignment;
}

void 
SMemInitialize(GameApplication* gameApp,
    uint64_t gameMemSize, uint64_t temporaryMemSize)
{
    GameMemSize = gameMemSize;
    TemporaryMemSize = temporaryMemSize;
    TotalMemoryAllocated = GameMemSize + TemporaryMemSize;

    void* gameMem = malloc(GameMemSize);
    SASSERT(gameMem);
    SMemClear(gameMem, GameMemSize);
    gameApp->GameMemory = CreateMemPoolFromBuffer(gameMem, GameMemSize);
    GameMemory = &gameApp->GameMemory;

    void* tempMem = malloc(TemporaryMemSize);
    SASSERT(tempMem);
    SMemClear(tempMem, TemporaryMemSize);
    gameApp->TemporaryMemory = CreateBiStackFromBuffer(tempMem, TemporaryMemSize);
    TempMemory = &gameApp->TemporaryMemory;

    SASSERT(GameMemory);
    SASSERT(TempMemory);

    SLOG_INFO("[ Memory ] Initialized! Total Mem: %d", TotalMemoryAllocated);
    SLOG_INFO("[ Memory ] Temporary mem size: %d", TemporaryMemSize);
    SLOG_INFO("[ Memory ] Game mem size: %d", GameMemSize);
}

void* SMemAlloc(size_t size)
{
    void* mem = MemPoolAlloc(GameMemory, size);
    SASSERT(mem);
    return mem;
}

void* SMemRealloc(void* block, size_t size)
{
    void* mem = MemPoolRealloc(GameMemory, block, size);
    SASSERT(mem);
    return mem;
}

void SMemFree(void* block)
{
    MemPoolFree(GameMemory, block);
}

void* SMemStdAlloc(size_t size)
{
    size_t alignedSize = AlignSize(size, sizeof(void*));
    return malloc(alignedSize);
}

void* SMemStdRealloc(void* block, size_t size)
{
    size_t alignedSize = AlignSize(size, sizeof(void*));
    void* mem = realloc(block, alignedSize);
    SASSERT(mem);
    return mem;
}

void SMemStdFree(void* block)
{
    free(block);
}

void* SMemAllocTag(size_t size, MemoryTag tag)
{
    SASSERT(tag != MemoryTag::Unknown);

    MemoryTagUsage[(uint8_t)tag] += size;
    TotalUsage += size;
    return SMemAlloc(size);
}

void* SMemReallocTag(void* block, size_t oldSize, size_t newSize, MemoryTag tag)
{
    SASSERT(tag != MemoryTag::Unknown);

    MemoryTagUsage[(uint8_t)tag] -= oldSize;
    MemoryTagUsage[(uint8_t)tag] += newSize;
    TotalUsage -= oldSize;
    TotalUsage += newSize;
    return SMemRealloc(block, newSize);
}

void SMemFreeTag(void* block, size_t size, MemoryTag tag)
{
    SASSERT(block);
    SASSERT(tag != MemoryTag::Unknown);

    MemoryTagUsage[(uint8_t)tag] -= size;
    TotalUsage -= size;
    SMemFree(block);
}

void SMemCopy(void* dst, const void* src, size_t size)
{
    memcpy(dst, src, size);
}

void SMemSet(void* block, int value, size_t size)
{
    memset(block, value, size);
}

void SMemClear(void* block, size_t size)
{
    memset(block, 0, size);
}

void SMemCopyAligned(void* dst, const void* src, size_t size, size_t alignment)
{
    size_t alignedSize = AlignSize(size, alignment);
    memcpy(dst, src, alignedSize);
}

void SMemSetAligned(void* block, int value, size_t size, size_t alignment)
{
    size_t alignedSize = AlignSize(size, alignment);
    memset(block, value, alignedSize);
}

void SMemClearAligned(void* block, size_t size, size_t alignment)
{
    size_t alignedSize = AlignSize(size, alignment);
    memset(block, 0, alignedSize);
}

const size_t* SMemGetTaggedUsages()
{
    return MemoryTagUsage;
}

size_t SMemGetUsage()
{
    return TotalUsage;
}

uint64_t SMemGetAllocated()
{
    return TotalMemoryAllocated;
}

global_var uint32_t NewUsageCount;

uint32_t GetNewCalls()
{
    return NewUsageCount;
}

//void* operator new(size_t size)
//{
//	++NewUsageCount;
//	return malloc(size);
//}
//
//void operator delete(void* mem)
//{
//	free(mem);
//}
//
//void* operator new[](size_t size)
//{
//	++NewUsageCount;
//	return malloc(size);
//}
//
//void operator delete[](void* mem)
//{
//	free(mem);
//}
