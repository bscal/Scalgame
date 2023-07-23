#include "STable.h"

#include "Core/SMemory.h"

struct TableBucket
{
	void* Value;
	uint32_t Key;
};

#define STABLE_HASH(key, size) (key % size)

STable* Alloc(int bucketCount, int bucketCapacity, int valueStride)
{
	STable* table = (STable*)SAlloc(ALLOC_GAME, sizeof(STable) + sizeof(SArray) * bucketCount, MemoryTag::Tables);
	table->BucketCount = bucketCount;
	table->ElementCount = 0;
	table->ValueStride = valueStride;

	for (int i = 0; i < bucketCount; ++i)
	{
		table->Buckets[i] = ArrayCreate((uint8_t)ALLOC_GAME, bucketCapacity, sizeof(TableBucket));
	}

	return table;
}

void* STableInsert(STable* table, uint32_t key, void* value)
{
	uint32_t idx = STABLE_HASH(key, table->BucketCount);
	SArray* bucket = &table->Buckets[idx];

	TableBucket entry;
	entry.Key = key;
	entry.Value = SAlloc(ALLOC_GAME, table->ValueStride, MemoryTag::Tables);
	ArrayPush(bucket, &entry);

	return ArrayPeekAt(bucket, bucket->Count - 1);
}

void* STableGet(STable* table, uint32_t key)
{
	uint32_t idx = STABLE_HASH(key, table->BucketCount);
	SArray* bucket = &table->Buckets[idx];
	for (uint32_t i = 0; i < bucket->Count; ++i)
	{
		TableBucket* entry = (TableBucket*)ArrayPeekAt(bucket, i);
		if (entry->Key == key)
		{
			return entry->Value;
		}
	}
	return nullptr;
}

bool STableContains(STable* table, uint32_t key)
{
	uint32_t idx = STABLE_HASH(key, table->BucketCount);
	SArray* bucket = &table->Buckets[idx];
	for (uint32_t i = 0; i < bucket->Count; ++i)
	{
		TableBucket* entry = (TableBucket*)ArrayPeekAt(bucket, i);
		if (entry->Key == key)
		{
			return true;
		}
	}
	return false;
}

void STableForEach(STable* table, void(*CB)(void*, void*), void* stack)
{
	for (int iBucket = 0; iBucket < table->BucketCount; ++iBucket)
	{
		SArray* bucketArr = &table->Buckets[iBucket];
		for (int i = 0; i > bucketArr->Count; ++i)
		{
			TableBucket* bucket = (TableBucket*)ArrayPeekAt(bucketArr, i);
			CB(bucket->Value, stack);
		}
	}
}