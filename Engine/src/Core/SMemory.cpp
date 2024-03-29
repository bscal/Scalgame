#include "SMemory.h"

#include "Game.h"
#include "SUtil.h"

#define RMEM_IMPLEMENTATION
#include "rmem/rmem.h"

//#include <rpmalloc/rpmalloc.h>
//#include <rpnew.h>

#if 0
#define SMEM_LOG_ALLOC(type, size) SLOG_INFO("[ Memory ] %s %d bytes, %d mb", type, size, size / 1024 / 1024)
#define SMEM_LOG_FREE() SLOG_INFO("[ Memory ] Freeing")
#else
#define SMEM_LOG_ALLOC(type, size)
#define SMEM_LOG_FREE()
#endif

// TODO: maybe move this to an internal state struct?
global_var MemPool* GameMemoryPtr;
global_var BiStack* TemporaryMemoryPtr;
global_var uint8_t* GameMemoryStart;
global_var uint8_t* TempMemoryStart;
global_var uint64_t MemoryTagUsage[(uint8_t)MemoryTag::MaxTags];
global_var uint64_t GameMemSize;
global_var uint64_t TemporaryMemSize;
global_var uint64_t TotalMemoryAllocated;
global_var uint64_t LastFrameTempMemoryUsage;

internal void* CMemAlloc(size_t n, size_t sz) { return SMemAlloc(n * sz); }

void
SMemInitialize(GameApplication* gameApp,
	size_t gameMemSize, size_t temporaryMemSize)
{
	GameMemSize = AlignSize(gameMemSize, 64);
	TemporaryMemSize = AlignSize(temporaryMemSize, 64);
	TotalMemoryAllocated = GameMemSize + TemporaryMemSize;

	GameMemoryStart = (uint8_t*)_aligned_malloc(TotalMemoryAllocated, 64);
	SASSERT(GameMemoryStart);
	SMemClear(GameMemoryStart, TotalMemoryAllocated);

	gameApp->GameMemory = CreateMemPoolFromBuffer(GameMemoryStart, GameMemSize);

	TempMemoryStart = GameMemoryStart + GameMemSize;
	gameApp->TemporaryMemory = CreateBiStackFromBuffer(TempMemoryStart, TemporaryMemSize);


	GameMemoryPtr = &gameApp->GameMemory;
	TemporaryMemoryPtr = &gameApp->TemporaryMemory;

	// Sets Raylibs RL memory allocator functions
	// Raylib usually doesnt use too much memory,
	// mostly loading of assets and files
	SetRLMalloc(SMemAlloc);
	SetRLCalloc(CMemAlloc);
	SetRLRealloc(SMemRealloc);
	SetRLFree(SMemFree);

	SASSERT(GameMemoryPtr);
	SASSERT(TemporaryMemoryPtr);
	SASSERT(gameApp->GameMemory.arena.mem);
	SASSERT(gameApp->TemporaryMemory.mem);
	SASSERT(TotalMemoryAllocated > 0);

	SLOG_INFO("[ Memory ] Initialized! Total Mem: %d bytes.", TotalMemoryAllocated);

	MemorySizeData gameFormatSize = FindMemSize(gameMemSize);
	SLOG_INFO("[ Memory ] Game mem size: %.2f%c. At: 0x%p", gameFormatSize.Size,
		gameFormatSize.BytePrefix, GameMemoryStart);

	MemorySizeData tempFormatSize = FindMemSize(temporaryMemSize);
	SLOG_INFO("[ Memory ] Temporary mem size: %.2f%c. At: 0x%p", tempFormatSize.Size,
		tempFormatSize.BytePrefix, TempMemoryStart);
}

void
SMemShutdown(GameApplication* gameApp)
{

}

void* SMemAlloc(size_t size)
{
	void* mem = MemPoolAlloc(GameMemoryPtr, size);
	SASSERT(mem);

	SMEM_LOG_ALLOC("Allocated", size);
	return mem;
}

void* SMemRealloc(void* block, size_t size)
{
	void* mem = MemPoolRealloc(GameMemoryPtr, block, size);
	SASSERT(mem);

	SMEM_LOG_ALLOC("Reallocated", size);
	return mem;
}

void SMemFree(void* block)
{
	MemPoolFree(GameMemoryPtr, block);
	SMEM_LOG_FREE();
}

void* SMemTempAlloc(size_t size)
{	
	void* ptr = BiStackAllocFront(TemporaryMemoryPtr, size);
	SASSERT(ptr);
	SMemClear(ptr, size);
	return ptr;
}

void SMemTempReset()
{
	SASSERT(TemporaryMemoryPtr);
	LastFrameTempMemoryUsage = TemporaryMemoryPtr->front - TemporaryMemoryPtr->mem;
	BiStackResetFront(TemporaryMemoryPtr);
}

void* SMemAllocTag(uint8_t allocator, size_t size, MemoryTag tag)
{
	SASSERT(size > 0);
	SASSERT(tag != MemoryTag::Unknown);

	void* memory;
	switch (allocator)
	{
		case((uint8_t)SAllocator::Game):
		{
			#if SMEM_USE_TAGS
			MemoryTagUsage[(uint8_t)tag] += size;
			#endif
			memory = SMemAlloc(size);
		} break;

		case((uint8_t)SAllocator::Temp):
		{
			memory = SMemTempAlloc(size);
		} break;

		case((uint8_t)SAllocator::Malloc):
		{
			#if SMEM_USE_TAGS
			MemoryTagUsage[(uint8_t)MemoryTag::TrackedMalloc] += size;
			#endif
			memory = _aligned_malloc(size, 16);
		} break;

		default:
		{
			memory = nullptr;
			SLOG_ERR("Using an invalid allocator!");
			SASSERT(false);
		} break;
	}
	SASSERT(memory);
	return memory;
}

void* SMemReallocTag(uint8_t allocator, void* ptr, size_t oldSize, size_t newSize, MemoryTag tag)
{
	SASSERT(newSize > 0);
	SASSERT(tag != MemoryTag::Unknown);

	void* memory;
	switch (allocator)
	{
		case((uint8_t)SAllocator::Game):
		{
			#if SMEM_USE_TAGS
			MemoryTagUsage[(uint8_t)tag] -= oldSize;
			MemoryTagUsage[(uint8_t)tag] += newSize;
			#endif
			memory = SMemRealloc(ptr, newSize);
		} break;

		case((uint8_t)SAllocator::Temp):
		{
			memory = SMemTempAlloc(newSize);
			if (ptr)
				SMemCopy(memory, ptr, oldSize);
		} break;

		case((uint8_t)SAllocator::Malloc):
		{
			#if SMEM_USE_TAGS
			MemoryTagUsage[(uint8_t)MemoryTag::TrackedMalloc] -= oldSize;
			MemoryTagUsage[(uint8_t)MemoryTag::TrackedMalloc] += newSize;
			#endif
			memory = _aligned_realloc(ptr, newSize, 16);
		} break;

		default:
		{
			memory = nullptr;
			SLOG_ERR("Using an invalid allocator!");
			SASSERT(false);
		} break;
	}
	return memory;
}

void  SMemFreeTag(uint8_t allocator, void* ptr, size_t size, MemoryTag tag)
{
	SASSERT(allocator < (uint8_t)SAllocator::MaxTypes);
	SASSERT(ptr);
	SASSERT(size > 0);
	SASSERT(tag != MemoryTag::Unknown);

	switch (allocator)
	{
		case((uint8_t)SAllocator::Game):
		{
#if SMEM_USE_TAGS
			MemoryTagUsage[(uint8_t)tag] -= size;
#endif
			SMemFree(ptr);
		} break;

		case((uint8_t)SAllocator::Malloc):
		{
			if (!ptr || size == 0)
			{
				SLOG_ERR("Using invalid ptr or size in SMemFreeTag -> SAllocator::Malloc. ptr: %p, size: %u"
					, ptr, size);
				break;
			}
#if SMEM_USE_TAGS
			MemoryTagUsage[(uint8_t)MemoryTag::TrackedMalloc] -= size;
#endif
			_aligned_free(ptr);
		} break;

		case((uint8_t)SAllocator::Temp):
			break;

		default:
		{
			SFATAL("Using invalid SAllocator in SMemFreeTag! Allocator: %u", allocator);
		} break;
	}
}

void* SMemAllocTagPrint(uint8_t allocator, size_t size, MemoryTag tag, int line, const char* file, const char* function)
{
	SASSERT(size > 0);
	SASSERT(tag != MemoryTag::Unknown);

	SLOG_DEBUG("[ Memory ] Allocating %d bytes. Allocator = %u, Tag = %u. \nFile: %s \nFunction: %s \nLine: %d",
		size, allocator, (uint8_t)tag, file, function, line);

	return SMemAllocTag(allocator, size, tag);
}

void* SMemReallocTagPrint(uint8_t allocator, void* ptr, size_t oldSize, size_t newSize, MemoryTag tag, int line, const char* file, const char* function)
{
	SASSERT(newSize > 0);
	SASSERT(tag != MemoryTag::Unknown);

	SLOG_DEBUG("[ Memory ] Reallocating %d bytes at %p. Allocator = %u, Tag = %u. \nFile: %s \nFunction: %s \nLine: %d",
		newSize, ptr, allocator, (uint8_t)tag, file, function, line);

	return SMemReallocTag(allocator, ptr, oldSize, newSize, tag);
}

void  SMemFreeTagPrint(uint8_t allocator, void* ptr, size_t size, MemoryTag tag, int line, const char* file, const char* function)
{
	SASSERT(tag != MemoryTag::Unknown);

	SLOG_DEBUG("[ Memory ] Freeing %d bytes at %p. Allocator = %u, Tag = %u. \nFile: %s \nFunction: %s \nLine: %d",
		size, ptr, allocator, (uint8_t)tag, file, function, line);

	return SMemFreeTag(allocator, ptr, size, tag);
}

bool ValidateMemory(SAllocator allocator, void* block)
{
	switch (allocator)
	{
		case(SAllocator::Game):
		{
			return ValidateGameMemory(block);
		} break;

		case(SAllocator::Temp):
		{
			return ValidateTempMemory(block);
		} break;

		case(SAllocator::Malloc):
		{
			return (block);
		} break;

		default:
		{
			SLOG_ERR("Using an invalid allocator!");
			SASSERT(false);
			return false;
		} break;
	}
}

bool ValidateGameMemory(void* block)
{
	return ((uint8_t*)(block) >= GameMemoryStart && (uint8_t*)(block) < (GameMemoryStart + GameMemSize));
}

bool ValidateTempMemory(void* block)
{
	return ((uint8_t*)(block) >= TempMemoryStart && (uint8_t*)(block) < (TempMemoryStart + TemporaryMemSize));
}

const size_t* SMemGetTaggedUsages()
{
	return MemoryTagUsage;
}

uint64_t SMemGetAllocated()
{
	return TotalMemoryAllocated;
}

uint64_t SMemGetLastFrameTempUsage()
{
	return LastFrameTempMemoryUsage;
}

// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
	SLOG_INFO("[ Memory ] " "new called, size %u", sz);
	if (sz == 0)
		++sz;

	void* block = _aligned_malloc(sz, 16);
	SASSERT(block);
	return block;
}

void* operator new[](std::size_t sz)
{
	SLOG_INFO("[ Memory ] " "new[] called, size %u", sz);
	if (sz == 0)
		++sz;

	void* block = _aligned_malloc(sz, 16);
	SASSERT(block);
	return block;
}

void operator delete(void* ptr) noexcept
{
	SLOG_INFO("[ Memory ] " "delete called");
	SASSERT(ptr);
	_aligned_free(ptr);
}

void operator delete[](void* ptr) noexcept
{
	SLOG_INFO("[ Memory ] " "delete[] called");
	SASSERT(ptr);
	_aligned_free(ptr);
}