#pragma once

#include "Core.h"

namespace Scal
{

void* MemAlloc(size_t size);
void* MemAllocZero(size_t size);
void* MemRealloc(void* block, size_t size);
void MemCopy(void* dst, const void* src, size_t size);
void MemClear(void* block, size_t size);
void MemFree(void* block);

}
