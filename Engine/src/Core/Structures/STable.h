#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define STABLE_DEFAULT_CAPACITY 10

global_var constexpr uint64_t STABLE_FULL = UINT64_MAX;

struct STableEntry
{
	void* Key;
	void* Value;
};


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
