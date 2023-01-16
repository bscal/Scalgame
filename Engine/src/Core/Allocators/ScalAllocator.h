#pragma once

#include "Core/Core.h"
#include "Core/Structures/STable.h"

#include "rmem/rmem.h"

struct SAllocator
{
	uint64_t Capacity;
	void* Memory;
};

struct SAllocatorBlock
{
	uint32_t Capacity;
	uint32_t Usage;
	void* Memory;
};

struct SAllocatorPool
{
	STable<uint16_t, SAllocatorBlock> Blocks;
};

struct SPooledAllocator
{
	ObjPool CreatueAllocator;
};



