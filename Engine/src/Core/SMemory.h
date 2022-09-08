#pragma once

#include "Core.h"

namespace Scal
{
namespace Memory
{
void* Alloc(size_t size);
void* AllocZero(size_t size);
void* ReAlloc(void* block, size_t size);
void Copy(void* dst, void* src, size_t size);
void Clear(void* block, size_t size);
void Free(void* block);
}
}


