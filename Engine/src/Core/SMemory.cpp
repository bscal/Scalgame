#include "SMemory.h"

#include "Core.h"
#include "Game.h"
#include "SUtil.h"

#define RMEM_IMPLEMENTATION
#include "rmem/rmem.h"

#include <rpmalloc.h>
//#include <rpnew.h>

#include <stdlib.h>

#if 0
#define SMEM_LOG_ALLOC(type, size) SLOG_INFO("[ Memory ] %s %d bytes, %d mb", type, size, size / 1024 / 1024)
#define SMEM_LOG_FREE() SLOG_INFO("[ Memory ] Freeing")
#else
#define SMEM_LOG_ALLOC(type, size)
#define SMEM_LOG_FREE()
#endif

// TODO: maybe move this to an internal state struct?
global_var GameApplication* GameAppPtr;
global_var uint64_t MemoryTagUsage[(uint8_t)MemoryTag::MaxTags];
global_var uint64_t GameMemSize;
global_var uint64_t TemporaryMemSize;
global_var uint64_t TotalMemoryAllocated;

internal void* CMemAlloc(size_t n, size_t sz) { return SMemAlloc(n * sz); }

void 
SMemInitialize(GameApplication* gameApp,
    size_t gameMemSize, size_t temporaryMemSize)
{
    GameAppPtr = gameApp;
    GameMemSize = gameMemSize;
    TemporaryMemSize = temporaryMemSize;
    TotalMemoryAllocated = GameMemSize + TemporaryMemSize;

    uint8_t* memoryLocation = (uint8_t*)malloc(TotalMemoryAllocated);
    SASSERT(memoryLocation);
    SMemClear(memoryLocation, TotalMemoryAllocated);

    gameApp->GameMemory = CreateMemPoolFromBuffer(memoryLocation, GameMemSize);
    SASSERT(gameApp->GameMemory.arena.mem);

    gameApp->TemporaryMemory = CreateBiStackFromBuffer(memoryLocation + GameMemSize, TemporaryMemSize);
    SASSERT(gameApp->TemporaryMemory.mem);

    int status = rpmalloc_initialize();

    // Sets Raylibs RL memory allocator functions
    // Raylib usually doesnt use too much memory,
    // mostly loading of assets and files
    SetRLMalloc(SMemAlloc);
    SetRLCalloc(CMemAlloc);
    SetRLRealloc(SMemRealloc);
    SetRLFree(SMemFree);

    SASSERT(GetRLMalloc());
    SASSERT(GetRLCalloc());
    SASSERT(GetRLRealloc());
    SASSERT(GetRLFree());

    SLOG_INFO("[ Memory ] Initialized! Total Mem: %d bytes.", TotalMemoryAllocated);

    MemorySizeData gameFormatSize = FindMemSize(gameMemSize);
    SLOG_INFO("[ Memory ] Game mem size: %.2f%c. At: 0x%p", gameFormatSize.Size,
        gameFormatSize.BytePrefix, memoryLocation);

    MemorySizeData tempFormatSize = FindMemSize(temporaryMemSize);
    SLOG_INFO("[ Memory ] Temporary mem size: %.2f%c. At: 0x%p", tempFormatSize.Size,
        tempFormatSize.BytePrefix, memoryLocation + GameMemSize);
}

void* SMemAlloc(size_t size)
{
    void* mem = MemPoolAlloc(&GameAppPtr->GameMemory, size);
    SASSERT(mem);

    SMEM_LOG_ALLOC("Allocated", size);
    return mem;
}

void* SMemRealloc(void* block, size_t size)
{
    void* mem = MemPoolRealloc(&GameAppPtr->GameMemory, block, size);
    SASSERT(mem);

    SMEM_LOG_ALLOC("Reallocated", size);
    return mem;
}

void SMemFree(void* block)
{
    MemPoolFree(&GameAppPtr->GameMemory, block);
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

void* SMemAllocTag(int allocator, size_t size, MemoryTag tag)
{
    SASSERT(size > 0);
    SASSERT(tag != MemoryTag::Unknown);
    
    void* memory;
    switch (allocator)
    {
        case(SAllocator::Game):
        {
            #if SMEM_USE_TAGS
                MemoryTagUsage[(uint8_t)tag] += size;
            #endif
            memory = SMemAlloc(size);
            break;
        };
        case(SAllocator::Temp):
        {
            memory = SMemTempAlloc(size);
            break;
        };
        default:
        {
            memory = nullptr;
            SLOG_ERR("Using an invalid allocator!");
            SASSERT(false);
        }
    }
    return memory;
}

void* SMemReallocTag(int allocator, void* ptr, size_t oldSize, size_t newSize, MemoryTag tag)
{
    SASSERT(newSize > 0);
    SASSERT(tag != MemoryTag::Unknown);

    void* memory;
    switch (allocator)
    {
        case(SAllocator::Game):
        {
            #if SMEM_USE_TAGS
                MemoryTagUsage[(uint8_t)tag] -= oldSize;
                MemoryTagUsage[(uint8_t)tag] += newSize;
            #endif
            memory = SMemRealloc(ptr, newSize);
            break;
        };
        case(SAllocator::Temp):
        {
            memory = SMemTempAlloc(newSize);
            SMemCopy(memory, ptr, oldSize);
            break;
        };
        default:
        {
            memory = nullptr;
            SLOG_ERR("Using an invalid allocator!");
            SASSERT(false);
        }
    }
    return memory;
}

void  SMemFreeTag(int allocator, void* ptr, size_t size, MemoryTag tag)
{
    SASSERT(tag != MemoryTag::Unknown);

    switch (allocator)
    {
        case(SAllocator::Game):
        {
            #if SMEM_USE_TAGS
                MemoryTagUsage[(uint8_t)tag] -= size;
            #endif
            SMemFree(ptr);
            break;
        };
        case(SAllocator::Temp):
        {
            break;
        };
        default:
        {
            SLOG_ERR("Using an invalid allocator!");
            SASSERT(false);
        }
    }
}

void SMemCopy(void* dst, const void* src, size_t size)
{
    SASSERT(dst);
    SASSERT(src);
    SASSERT(size > 0);
    SASSERT(dst != src);
    memcpy(dst, src, size);
}

void SMemMove(void* dst, const void* src, size_t size)
{
    SASSERT(dst);
    SASSERT(src);
    SASSERT(size > 0);
    SASSERT(dst != src);
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

const size_t* SMemGetTaggedUsages()
{
    return MemoryTagUsage;
}

uint64_t SMemGetAllocated()
{
    return TotalMemoryAllocated;
}

void* operator new(size_t sz)
{
    SLOG_INFO("[ Memory ] " "new called, size %u", sz);
    return malloc(sz);
}

void* operator new[](size_t sz)
{
    SLOG_INFO("[ Memory ] " "new[] called, size %u", sz);
    return malloc(sz);
}

void operator delete(void* ptr) noexcept
{
    SLOG_INFO("[ Memory ] " "delete called");
    free(ptr);
}

void operator delete[](void* ptr) noexcept
{
     SLOG_INFO("[ Memory ] " "delete[] called");
    free(ptr);
}