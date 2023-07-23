#pragma once

#include "Core/Core.h"

#include "SArray.h"

struct STable
{
	int BucketCount;
	int ElementCount;
	int ValueStride;
	SArray Buckets[0];
};

STable* Alloc(int bucketCount, int bucketCapacity, int valueStride);

void* STableInsert(STable* table, uint32_t key, void* value);

void* STableGet(STable* table, uint32_t key);

bool STableContains(STable* table, uint32_t key);

void STableForEach(STable* table, void(*CB)(void*, void*), void* stack);