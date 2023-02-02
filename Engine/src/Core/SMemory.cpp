#define RMEM_IMPLEMENTATION
#include "SMemory.h"

#include "Game.h"
#include "SUtil.h"

#include <stdlib.h>

// TODO: maybe move this to an internal state struct?
global_var struct MemPool* GameMemory;
global_var struct BiStack* TempMemory;
global_var uint64_t MemoryTagUsage[(uint8_t)MemoryTag::MaxTags];
global_var uint64_t TotalUsage;
global_var uint64_t GameMemSize;
global_var uint64_t TemporaryMemSize;
global_var uint64_t TotalMemoryAllocated;
global_var uint64_t TotalTempMemAllocated;

void 
SMemInitialize(GameApplication* gameApp,
    uint64_t gameMemSize, uint64_t temporaryMemSize)
{
    GameMemSize = gameMemSize;
    TemporaryMemSize = temporaryMemSize;
    TotalMemoryAllocated = GameMemSize + TemporaryMemSize;

    uint8_t* memory = (uint8_t*)malloc(TotalMemoryAllocated);
    SASSERT(memory);
    SMemClear(memory, TotalMemoryAllocated);

    gameApp->GameMemory = CreateMemPoolFromBuffer(memory, GameMemSize);
    GameMemory = &gameApp->GameMemory;

    gameApp->TemporaryMemory = CreateBiStackFromBuffer(memory + GameMemSize, TemporaryMemSize);
    TempMemory = &gameApp->TemporaryMemory;

    SASSERT(GameMemory->arena.mem);
    SASSERT(TempMemory->mem);

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

void* SMemTempAlloc(size_t size)
{
    TotalTempMemAllocated += size;
    return BiStackAllocFront(TempMemory, size);
}

void SMemTempReset()
{
    TotalTempMemAllocated = 0;
    BiStackResetFront(TempMemory);
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

void SMemMove(void* dst, const void* src, size_t size)
{
    memmove(dst, src, size);
}

void SMemSet(void* block, int value, size_t size)
{
    memset(block, value, size);
}

void SMemClear(void* block, size_t size)
{
    memset(block, 0, size);
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

uint64_t SMemGetTempAllocated()
{
    return TotalTempMemAllocated;
}

inline MemPool* const GetGameMemory()
{
    return GameMemory;
}

inline BiStack* const GetTempMemory()
{
    return TempMemory;
}

static int NewUsageCount;

int GetNewCalls()
{
    return NewUsageCount;
}

void* operator new(size_t size) noexcept(false)
{
    ++NewUsageCount;
    return SMemAlloc(size);
}

void operator delete(void* block) noexcept(false)
{
    SMemFree(block);
}

void* operator new[](size_t size) noexcept(false)
{
    ++NewUsageCount;
    return SMemAlloc(size);
}
void operator delete[](void* block) noexcept(false)
{
    SMemFree(block);
}
