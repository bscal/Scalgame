#define RMEM_IMPLEMENTATION
#include "SMemory.h"

#include "Core.h"
#include "Game.h"
#include "SUtil.h"

#include <stdlib.h>

#if SCAL_DEBUG
#define SMEM_LOG_ALLOC(type, size) SLOG_INFO("[ Memory ] %s %d bytes, %d mb", type, size, size / 1024 / 1024)
#define SMEM_LOG_FREE() SLOG_INFO("[ Memory ] Freeing")
#else
#define SMEM_LOG_ALLOC(type, size)
#define SMEM_LOG_FREE()
#endif

#define UseMalloc 0
#if UseMalloc
#define SMalloc(void) malloc(size)
#define SRealloc(void) realloc(block, size)
#define SFree(void) free(block)
#else
#define SMalloc(void) MemPoolAlloc(&GameAppPtr->GameMemory, size)
#define SRealloc(void) MemPoolRealloc(&GameAppPtr->GameMemory, block, size)
#define SFree(void) MemPoolFree(&GameAppPtr->GameMemory, block)
#endif

// TODO: maybe move this to an internal state struct?
global_var GameApplication* GameAppPtr;
global_var uint64_t MemoryTagUsage[(uint8_t)MemoryTag::MaxTags];
global_var uint64_t TotalUsage;
global_var uint64_t GameMemSize;
global_var uint64_t TemporaryMemSize;
global_var uint64_t TotalMemoryAllocated;

void 
SMemInitialize(GameApplication* gameApp,
    uint64_t gameMemSize, uint64_t temporaryMemSize)
{
    GameAppPtr = gameApp;
    GameMemSize = gameMemSize;
    TemporaryMemSize = temporaryMemSize;
    TotalMemoryAllocated = GameMemSize + TemporaryMemSize;

    uint8_t* memory = (uint8_t*)malloc(TotalMemoryAllocated);
    SASSERT(memory);
    SMemClear(memory, TotalMemoryAllocated);

    gameApp->GameMemory = CreateMemPoolFromBuffer(memory, GameMemSize);

    gameApp->TemporaryMemory = CreateBiStackFromBuffer(memory + GameMemSize, TemporaryMemSize);

    SASSERT(gameApp->GameMemory.arena.mem);
    SASSERT(gameApp->TemporaryMemory.mem);

    MemorySizeData gameFormatSize = FindMemSize(gameMemSize);
    MemorySizeData tempFormatSize = FindMemSize(temporaryMemSize);

    SLOG_INFO("[ Memory ] Initialized! Total Mem: %d bytes.", TotalMemoryAllocated);
    SLOG_INFO("[ Memory ] Game mem size: %.2f%c. At: 0x%p", gameFormatSize.Size,
        gameFormatSize.BytePrefix, memory);
    SLOG_INFO("[ Memory ] Temporary mem size: %.2f%c. At: 0x%p", tempFormatSize.Size,
        tempFormatSize.BytePrefix, memory + GameMemSize);
}

void* SMemAlloc(size_t size)
{
    void* mem = SMalloc();
    //SMEM_LOG_ALLOC("Allocated", size);
    SASSERT(mem);
    return mem;
}

void* SMemRealloc(void* block, size_t size)
{
    void* mem = SRealloc();
    //SMEM_LOG_ALLOC("Reallocated", size);
    SASSERT(mem);
    return mem;
}

void SMemFree(void* block)
{
    SFree();
    SMEM_LOG_FREE();
}

void* SMemTempAlloc(size_t size)
{
    return BiStackAllocFront(&GameAppPtr->TemporaryMemory, size);
}

void SMemTempReset()
{
    GameAppPtr->LastFrameTempMemoryUsage = GameAppPtr->TemporaryMemory.front - GameAppPtr->TemporaryMemory.mem;
    BiStackResetFront(&GameAppPtr->TemporaryMemory);
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

void* operator new(size_t sz)
{
    return SMemAlloc(sz);
}

void* operator new[](size_t sz)
{
    return SMemAlloc(sz);
}

void operator delete(void* ptr) noexcept
{
    SMemFree(ptr);
}

void operator delete[](void* ptr) noexcept
{
    SMemFree(ptr);
}