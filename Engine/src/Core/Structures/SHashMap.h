#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

constexpr global_var uint32_t HASHMAP_RESIZE = 2u;
constexpr global_var uint32_t HASHMAP_NOT_FOUND = UINT32_MAX;
constexpr global_var float HASHMAP_LOAD_FACTOR = 0.8f;

template<typename K, typename V>
struct SHashMapBucket
{
	K Key;
	V Value;
	uint32_t ProbeLength : 31;
	uint32_t Occupied : 1;
};

// Hashmap using RobinHood open addressing and power of 2 capacties for faster modulo.
template<typename K, typename V, typename Hasher = DefaultHasher>
struct SHashMap
{
	SHashMapBucket<K, V>* Buckets;
	uint32_t Capacity;
	uint32_t Size;
	uint32_t MaxSize;
	SAllocator Allocator;

	void Reserve(uint32_t newCapacity);
	void Clear();
	void Free();

	uint32_t Insert(const K* key, const V* val);	// Inserts Key/Value
	V* InsertKey(const K* key);						// Inserts Key, returns ptr to value
	V* Get(const K* key) const;						// Returns ptr to value
	bool Remove(const K* key);
	bool RemoveValue(const K* key, V* value);

	_FORCE_INLINE_ V* Index(uint32_t i) { return (Buckets[i].Occupied) ? &Buckets[i].Value : nullptr; }
	_FORCE_INLINE_ bool IsAllocated() const { return Buckets; }
	_FORCE_INLINE_ void Foreach(void(*CB)(K*, V*, void*), void* ctx)
	{
		for (uint32_t i = 0; i < Capacity; ++i)
		{
			if (Buckets[i].Occupied)
			{
				CB(&Buckets[i].Key, &Buckets[i].Value, ctx);
			}
		}
	}

private:
	_FORCE_INLINE_ uint32_t Hash(const K* key) const;
	_ALWAYS_INLINE_ bool Equals(const K* k0, const K* k1) const { return *k0 == *k1; }

	uint32_t FindIndexAndInsert(SHashMapBucket<K, V>* swapBucket);
};

template<typename K, typename V, typename Hasher>
void SHashMap<K, V, Hasher>::Reserve(uint32_t capacity)
{
	constexpr size_t stride = sizeof(SHashMapBucket<K, V>);

	// Align capacity
	uint32_t newCapacity;
	if (capacity == 0)
		newCapacity = 2;
	else if (!IsPowerOf2_32(capacity))
		newCapacity = AlignPowTwo32Ceil(capacity);
	else
		newCapacity = capacity;

	if (newCapacity <= Capacity)
		return;

	uint32_t oldCapacity = Capacity;
	size_t oldSize = oldCapacity * stride;
	size_t newSize = newCapacity * stride;

	Capacity = newCapacity;
	MaxSize = (uint32_t)((float)Capacity * HASHMAP_LOAD_FACTOR);

	if (Size == 0)
	{
		Buckets = (SHashMapBucket<K, V>*)(SRealloc(Allocator, Buckets, oldSize, newSize, MemoryTag::Tables));
	}
	else
	{
		SHashMap<K, V, Hasher> tmpTable;
		tmpTable.Capacity = Capacity;
		tmpTable.MaxSize = MaxSize;
		tmpTable.Size = 0;
		tmpTable.Allocator = Allocator;
		tmpTable.Buckets = (SHashMapBucket<K, V>*)SAlloc(tmpTable.Allocator, newSize, MemoryTag::Tables);

		for (uint32_t i = 0; i < oldCapacity; ++i)
		{
			if (Buckets[i].Occupied)
				tmpTable.Insert(&Buckets[i].Key, &Buckets[i].Value);

			if (tmpTable.Size == Size) 
				break;
		}

		SFree(Allocator, Buckets, oldSize, MemoryTag::Tables);
		SASSERT(Size == tmpTable.Size);
		SLOG_DEBUG("SHashMap resized! From: %u, To: %u", oldCapacity, newCapacity);
		*this = tmpTable;
	}

	SASSERT(Buckets);
	SASSERT(Capacity > 0);
	SASSERT(IsPowerOf2_32(Capacity));
}

template<typename K, typename V, typename Hasher>
void 
SHashMap<K, V, Hasher>::Clear()
{
	SMemClear(Buckets, Capacity * sizeof(SHashMapBucket<K, V>));
	Size = 0;
}

template<typename K, typename V, typename Hasher>
void 
SHashMap<K, V, Hasher>::Free()
{
	SFree(Allocator, Buckets, Capacity * sizeof(SHashMapBucket<K, V>), MemoryTag::Tables);
	Buckets = nullptr;
	Capacity = 0;
	Size = 0;
	MaxSize = 0;
}

template<typename K, typename V, typename Hasher>
uint32_t SHashMap<K, V, Hasher>::Insert(const K* key, const V* value)
{
	SASSERT(key);
	SASSERT(value);

	if (Size >= MaxSize)
	{
		Reserve(Capacity * HASHMAP_RESIZE);
	}

	SASSERT(Buckets);

	SHashMapBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Value = *value;
	swapBucket.Occupied = true;

	uint32_t index = FindIndexAndInsert(&swapBucket);
	SASSERT(index != HASHMAP_NOT_FOUND);
	return index;
}

template<typename K, typename V, typename Hasher>
V* SHashMap<K, V, Hasher>::InsertKey(const K* key)
{
	SASSERT(key);

	if (!key)
		return nullptr;

	if (Size >= MaxSize)
	{
		Reserve(Capacity * HASHMAP_RESIZE);
	}

	SASSERT(Buckets);

	SHashMapBucket<K, V> swapBucket = {};
	swapBucket.Key = *key;
	swapBucket.Occupied = true;

	uint32_t index = FindIndexAndInsert(&swapBucket);
	SASSERT(index != HASHMAP_NOT_FOUND);
	SASSERT(index < Capacity);
	return &Buckets[index].Value;
}

template<typename K, typename V, typename Hasher>
V* SHashMap<K, V, Hasher>::Get(const K* key) const
{
	SASSERT(key);

	if (!IsAllocated())
		return nullptr;

	uint32_t probeLength = 0;
	uint32_t index = Hash(key);
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 0 || probeLength > bucket->ProbeLength)
			return nullptr;
		else if (Equals(key, &bucket->Key))
			return &bucket->Value;
		else
		{
			++probeLength;
			++index;
			if (index == Capacity)
				index = 0;
		}
	}
	return nullptr;
}

template<typename K, typename V, typename Hasher>
bool SHashMap<K, V, Hasher>::Remove(const K* key)
{
	SASSERT(IsAllocated());
	SASSERT(key);

	uint32_t index = Hash(key);
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied)
		{
			if (Equals(key, &bucket->Key))
			{
				while (true) // Move any entries after index closer to their ideal probe length.
				{
					uint32_t lastIndex = index;
					if (++index == Capacity)
						index = 0;

					SHashMapBucket<K, V>* nextBucket = &Buckets[index];
					if (!nextBucket->Occupied || nextBucket->ProbeLength == 0) // No more entires to move
					{
						Buckets[lastIndex].ProbeLength = 0;
						Buckets[lastIndex].Occupied = false;
						--Size;
						return true;
					}
					else
					{
						--nextBucket->ProbeLength;
						Buckets[lastIndex] = *nextBucket;
					}
				}
			}
			else
			{
				if (++index == Capacity)
					index = 0; // continue searching till 0 or found equals key
			}
		}
		else
		{
			break; // No key found
		}
	}
	return false;
}

template<typename K, typename V, typename Hasher>
bool SHashMap<K, V, Hasher>::RemoveValue(const K* key, V* valuePtr)
{
	SASSERT(IsAllocated());
	SASSERT(key);

	uint32_t index = Hash(key);
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied)
		{
			if (Equals(key, &bucket->Key))
			{
				*valuePtr = bucket->Value;
				while (true) // Move any entries after index closer to their ideal probe length.
				{
					uint32_t lastIndex = index;
					if (++index == Capacity)
						index = 0;

					SHashMapBucket<K, V>* nextBucket = &Buckets[index];
					if (!nextBucket->Occupied || nextBucket->ProbeLength == 0) // No more entires to move
					{
						Buckets[lastIndex].ProbeLength = 0;
						Buckets[lastIndex].Occupied = false;
						--Size;
						return true;
					}
					else
					{
						--nextBucket->ProbeLength;
						Buckets[lastIndex] = *nextBucket;
					}
				}
			}
			else
			{
				if (++index == Capacity)
					index = 0; // continue searching till 0 or found equals key
			}
		}
		else
		{
			break; // No key found
		}
	}
	return false;
}

template<typename K, typename V, typename Hasher>
_FORCE_INLINE_ uint32_t
SHashMap<K, V, Hasher>::Hash(const K* key) const
{
	SASSERT(IsPowerOf2_32(Capacity));
	uint32_t hash = Hasher{}(key, sizeof(K));
	// Fast mod of power of 2s
	hash &= (Capacity - 1);
	// Since we mod by capacity is will not be larger then capacity and cast to u32
	return hash;
}

template<typename K, typename V, typename Hasher>
uint32_t
SHashMap<K, V, Hasher>::FindIndexAndInsert(SHashMapBucket<K, V>* swapBucket)
{
	SASSERT(swapBucket);

	uint32_t insertIndex = HASHMAP_NOT_FOUND;

	uint32_t index = Hash(&swapBucket->Key);
	uint32_t probeLength = 0;
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied) // Bucket is being used
		{
			// Duplicate
			if (Equals(&bucket->Key, &swapBucket->Key))
				return index;

			// Swaps swapBucket and larger element
			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				swapBucket->ProbeLength = probeLength;
				probeLength = bucket->ProbeLength;
				SHashMapBucket<K, V> temp;
				temp = *bucket;
				*bucket = *swapBucket;
				*swapBucket = temp;

				// We want to store pointer to our inserted element
				if (insertIndex == HASHMAP_NOT_FOUND)
					insertIndex = index;
			}

			++probeLength;
			// Continues searching
			if (++index == Capacity)
				index = 0;
		}
		else
		{
			// Found an open spot. Insert and stops searching
			*bucket = *swapBucket;
			bucket->ProbeLength = probeLength;
			++Size;

			// We want to store pointer to our inserted element
			if (insertIndex == HASHMAP_NOT_FOUND)
				insertIndex = index;
			
			// Exit's loop and properly returns result
			break;
		}
	}
	return insertIndex;
}

inline int TestSHoodTable()
{
	SHashMap<int, int> table = {};
	SASSERT(!table.IsAllocated());

	int k0 = 5;
	int v1 = 255;
	table.Insert(&k0, &v1);

	SASSERT(table.Size == 1);
	SASSERT(table.MaxSize == uint32_t((float)table.Capacity * HASHMAP_LOAD_FACTOR));
	SASSERT(table.Capacity == 2);

	int* get0 = table.Get(&k0);
	SASSERT(get0);
	SASSERT(*get0 == v1);

	for (int i = 0; i < 17; ++i)
	{
		int ii = i * 2;
		table.Insert(&i, &ii);
	}

	for (int i = 0; i < 17; ++i)
	{
		SASSERT(table.Get(&i));
	}

	SASSERT(table.Size == 17);
	SASSERT(table.MaxSize == uint32_t((float)table.Capacity * HASHMAP_LOAD_FACTOR));
	SASSERT(table.Capacity == 32);

	int* get1 = table.Get(&k0);
	SASSERT(get1);
	SASSERT(*get1 == v1);

	table.Remove(&k0);

	int remove = 9;
	table.Remove(&remove);

	int remove2 = 9;
	table.Remove(&remove2);

	SASSERT(!table.Get(&k0));
	SASSERT(table.Size == 15);

	for (int i = 0; i < 4; ++i)
	{
		int* get2 = table.Get(&i);
		SASSERT(get2);
		SASSERT(*get2 == i * 2);
	}

	int ik = 100;
	int* ikv = table.InsertKey(&ik);
	*ikv = 101;
	SASSERT(table.Size == 16);
	int* ikvget = table.Get(&ik);
	SASSERT(*ikvget == 101);

	table.Free();
	SASSERT(!table.Buckets);
	SASSERT(table.Capacity == 0);

	return 1;
}
