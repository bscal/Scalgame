#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

#include <functional>

constexpr static uint32_t SHOOD_RESIZE = 2u;
constexpr static float SHOOD_LOAD_FACTOR = 1.75f;
constexpr static float SHOOD_LOAD_FACTOR_INVERSE = SHOOD_LOAD_FACTOR - 1.0f;

template<typename K, typename V>
struct SHashMapBucket
{
	K Key;
	V Value;
	uint32_t ProbeLength : 31;
	uint32_t Occupied : 1;
};

// Hashmap using RobinHood open addressing and power of 2 capacties for faster modulo.
template<typename K, typename V, typename HashFunc = DefaultHasher<K>, typename EqualsFunc = DefaultEquals<K>>
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

	void Insert(const K* key, const V* val);						// Inserts Key/Value
	SHashMapBucket<K, V>* InsertAndGet(const K* key, const V* val);	// Inserts Key/Value returns bucket
	V* InsertKey(const K* key);										// Inserts Key, returns ptr to value
	V* Get(const K* key) const;										// Returns ptr to value
	bool Contains(const K* key) const;
	bool Remove(const K* key);

	_FORCE_INLINE_ bool IsAllocated() const { return Buckets; }
	_FORCE_INLINE_ size_t Stride() const { return sizeof(SHashMapBucket<K, V>); }
	_FORCE_INLINE_ size_t MemUsed() const { return Capacity * Stride(); }

	const SHashMapBucket<K, V>& operator[](size_t i) const { SASSERT(i < Capacity); return Buckets[i]; }
	SHashMapBucket<K, V>& operator[](size_t i) { SASSERT(i < Capacity); return Buckets[i]; }

	_FORCE_INLINE_ void Foreach(std::function<void(V*)> onElement)
	{
		for (uint32_t i = 0; i < Capacity; ++i)
		{
			if (Buckets[i].Occupied)
			{
				onElement(&Buckets[i].Value);
			}
		}
	}

private:
	_FORCE_INLINE_ uint64_t Hash(const K* key) const;

	SHashMapBucket<K, V>* FindIndexAndInsert(SHashMapBucket<K, V>* swapBucket);
};

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHashMap<K, V, HashFunc, EqualsFunc>::Reserve(uint32_t capacity)
{
	uint32_t newCapacity = (uint32_t)((float)capacity);
	if (newCapacity == 0)
		newCapacity = 2;
	else if (!IsPowerOf2_32(newCapacity))
		newCapacity = AlignPowTwo32Ceil(newCapacity);

	if (newCapacity <= Capacity)
		return;

	MaxSize = (uint32_t)((float)newCapacity * SHOOD_LOAD_FACTOR_INVERSE);
	uint32_t oldCapacity = Capacity;
	Capacity = newCapacity;

	if (Size == 0)
	{
		size_t oldSize = oldCapacity * Stride();
		size_t newSize = newCapacity * Stride();
		Buckets = (SHashMapBucket<K, V>*)(SRealloc(Allocator, Buckets, oldSize, newSize, MemoryTag::Tables));
	}
	else
	{
		SHashMap<K, V, HashFunc, EqualsFunc> tmpTable = {};
		tmpTable.Allocator = Allocator;
		tmpTable.Capacity = Capacity;
		tmpTable.MaxSize = MaxSize;
		tmpTable.Buckets = (SHashMapBucket<K, V>*)SAlloc(Allocator, newCapacity * Stride(), MemoryTag::Tables);

		for (uint32_t i = 0; i < oldCapacity; ++i)
		{
			if (Buckets[i].Occupied == 1)
			{
				tmpTable.Insert(&Buckets[i].Key, &Buckets[i].Value);
			}
			if (tmpTable.Size == Size) break;
		}

		SFree(Allocator, Buckets, oldCapacity * Stride(), MemoryTag::Tables);
		SASSERT(Size == tmpTable.Size);
		SLOG_DEBUG("SHashMap resized! From: %u, To: %u", oldCapacity, newCapacity);
		*this = tmpTable;
	}

	SASSERT(Buckets);
	SASSERT(Capacity > 0);
	SASSERT(IsPowerOf2_32(Capacity));
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHashMap<K, V, HashFunc, EqualsFunc>::Clear()
{
	SMemClear(Buckets, MemSize());
	Size = 0;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHashMap<K, V, HashFunc, EqualsFunc>::Free()
{
	SFree(Allocator, Buckets, MemUsed(), MemoryTag::Tables);
	Buckets = nullptr;
	Capacity = 0;
	Size = 0;
	MaxSize = 0;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHashMap<K, V, HashFunc, EqualsFunc>::Insert(const K* key, const V* value)
{
	SASSERT(key);
	SASSERT(value);
	SASSERT(EqualsFunc{}(key, key));

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	SHashMapBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Value = *value;
	swapBucket.Occupied = true;

	SHashMapBucket<K, V>* result = FindIndexAndInsert(&swapBucket);
	SASSERT(result);
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
SHashMapBucket<K, V>* SHashMap<K, V, HashFunc, EqualsFunc>::InsertAndGet(const K* key, const V* value)
{
	SASSERT(key);
	SASSERT(value);
	SASSERT(EqualsFunc{}(key, key));

	if (!key)
		return nullptr;

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	SHashMapBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Value = *value;
	swapBucket.Occupied = true;

	SHashMapBucket<K, V>* result = FindIndexAndInsert(&swapBucket);
	SASSERT(result);
	return result;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
V* SHashMap<K, V, HashFunc, EqualsFunc>::InsertKey(const K* key)
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(key, key));

	if (!key)
		return nullptr;

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	SHashMapBucket<K, V> swapBucket = {};
	swapBucket.Key = *key;
	swapBucket.Occupied = true;

	SHashMapBucket<K, V>* result = FindIndexAndInsert(&swapBucket);
	SASSERT(result);
	return &result->Value;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
V* SHashMap<K, V, HashFunc, EqualsFunc>::Get(const K* key) const
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(key, key));

	if (!IsAllocated())
		return nullptr;

	uint64_t hash = Hash(key);
	uint32_t index = (uint32_t)hash;
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 0)
			return nullptr;
		else if (EqualsFunc{}(&bucket->Key, key))
			return &bucket->Value;
		else
			if (++index == Capacity) index = 0;
	}
	return nullptr;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
bool SHashMap<K, V, HashFunc, EqualsFunc>::Contains(const K* key) const
{
	SASSERT(EqualsFunc{}(key, key));
	SASSERT(key);

	if (!IsAllocated())
		return false;

	uint64_t hash = Hash(key);
	uint32_t index = (uint32_t)hash;
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 0)
			return false;
		else if (EqualsFunc{}(&bucket->Key, key))
			return true;
		else
			if (++index == Capacity) index = 0;
	}
	return false;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
bool SHashMap<K, V, HashFunc, EqualsFunc>::Remove(const K* key)
{
	SASSERT(IsAllocated());
	SASSERT(EqualsFunc{}(key, key));
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint32_t index = (uint32_t)hash;
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied)
		{
			if (EqualsFunc{}(&bucket->Key, key))
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

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
_FORCE_INLINE_ uint64_t
SHashMap<K, V, HashFunc, EqualsFunc>::Hash(const K* key) const
{
	SASSERT(IsPowerOf2_32(Capacity));
	uint64_t hash = HashFunc{}(key);
	// Fast mod of power of 2s
	hash &= ((uint64_t)(Capacity - 1));
	// Since we mod by capacity is will not be larger then capacity and cast to u32
	SASSERT(hash < Capacity);
	return hash;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
SHashMapBucket<K, V>* 
SHashMap<K, V, HashFunc, EqualsFunc>::FindIndexAndInsert(SHashMapBucket<K, V>* swapBucket)
{
	SASSERT(swapBucket);

	SHashMapBucket<K, V>* result = nullptr;

	uint32_t index = Hash(&swapBucket->Key);
	uint32_t probeLength = 0;
	while (true)
	{
		SHashMapBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, &swapBucket->Key))
				return bucket;

			// Swaps swapBucket and larger element
			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				swapBucket->ProbeLength = probeLength;
				probeLength = bucket->ProbeLength;
				std::swap(*bucket, *swapBucket);

				// We want to store pointer to our inserted element
				if (!result)
					result = bucket;
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
			if (!result)
				result = bucket;
			
			// Exit's loop and properly returns result
			break;
		}
	}
	return result;
}

inline int TestSHoodTable()
{
	SHashMap<int, int> table = {};
	SASSERT(!table.IsAllocated());

	int k0 = 5;
	int v1 = 255;
	table.Insert(&k0, &v1);

	SASSERT(table.Size == 1);
	SASSERT(table.MaxSize == uint32_t((float)table.Capacity * SHOOD_LOAD_FACTOR_INVERSE));
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
		SASSERT(table.Contains(&i));
	}

	SASSERT(table.Size == 17);
	SASSERT(table.MaxSize == uint32_t((float)table.Capacity * SHOOD_LOAD_FACTOR_INVERSE));
	SASSERT(table.Capacity == 32);

	int* get1 = table.Get(&k0);
	SASSERT(get1);
	SASSERT(*get1 == v1);

	table.Remove(&k0);

	int remove = 9;
	table.Remove(&remove);

	int remove2 = 9;
	table.Remove(&remove2);

	SASSERT(!table.Contains(&k0));
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
