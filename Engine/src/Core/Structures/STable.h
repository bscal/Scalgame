#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define STABLE_DEFAULT_CAPACITY 10

global_var constexpr uint64_t STABLE_FULL = UINT64_MAX;

template<typename K, typename V>
struct STableEntry
{
	K Key;
	V Value;
	STableEntry<K, V>* Next;
};

template<typename K, typename V>
struct STable
{
	uint64_t Size;
	uint64_t Capacity;
	STableEntry<K, V>** Entries;
};

template<typename K, typename V>
void STableCreate(STable<K, V>* sTable, uint64_t capacity)
{
	if (!sTable)
	{
		TraceLog(LOG_ERROR, "sTable cannot be nullptr");
		return;
	}

	if (capacity < 1)
	{
		capacity = 1;
	}

	sTable->Size = 0;
	sTable->Capacity = capacity;
	sTable->Entries = (STableEntry<K, V>**)
		Scal::MemAllocZero(capacity * sizeof(STableEntry));

	assert(sTable->Entries);
}

template<typename K, typename V>
void STablePut(STable<K, V>* sTable, const K* key, const V* value)
{
	assert(sTable);
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return;
	}
	if (!value)
	{
		TraceLog(LOG_ERROR, "value cannot be null");
		return;
	}

	uint64_t bucket = 0;
	STableEntry<K, V>* entry = sTable->Entries[bucket];

	if (!entry) // If emptry insert
	{
		entry->Key = *key;
		entry->Value = *value;
		entry->Next = 0;
		sTable->Entries[bucket] = entry;
	}

	STableEntry<K, V>* previous;
	while (entry)
	{
		// TODO
		if (true) // see if equal
		{

		}

		previous = entry;
		entry = previous->Next;
	}

	entry->Key = *key;
	entry->Value = *value;
	entry->Next = 0;
	previous->Next = entry;
}

template<typename K, typename V>
V* STableGet(STable<K, V>* sTable, const K* key)
{
	return 0;
}


template<typename T>
struct Bucket
{
	uint64_t Hash;
	T Value;
	bool InUse;
};

template<typename K, typename V>
struct HashMap
{
	uint64_t Size;
	uint64_t Capacity;
	Bucket<V>* BucketMemory;
	uint64_t(*KeyHashFunc)(K key);

	bool Initialize();
	void Free();
	void Clear();
	void Rehash();

	uint64_t Hash(K key);

	void* Get();

	V* Put(K key, V value);
	void* Remove();
	bool Contains();

};

template<typename K, typename V>
bool HashMap<K, V>::Initialize()
{
	BucketMemory = (Bucket*)Scal::MemAlloc(sizeof(Bucket));
	assert(BucketMemory);

	Capacity = STABLE_DEFAULT_CAPACITY;
}

template<typename K, typename V>
uint64_t HashMap<K, V>::Hash(K key)
{
	/* If full, return immediately */
	if (Size == Capacity)
		return STABLE_FULL;

	/* Find the best index */
	uint64_t curr = KeyHashFunc(key);

	/* Linear probling */
	for (uint64_t i = 0; i < Size; ++i)
	{
		if (!BucketMemory[curr].IsUsed)
			return curr;

		if (BucketMemory[curr].Hash == key && BucketMemory[curr].IsUsed)
			return curr;

		curr = (curr + 1) % Size;
	}
	
	return STABLE_FULL
}

template<typename K, typename V>
V* HashMap<K, V>::Put(K key, V value)
{
	uint64_t index = Hash(key);
	while (index == STABLE_FULL)
	{
		Rehash();
		index = Hash(key);
	}

	BucketMemory[index].Hash = index;
	BucketMemory[index].Value = value;
	BucketMemory[index].IsUsed = true;
	++Size;
}
