#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

constexpr static uint32_t SHOOD_RESIZE = 2u;
constexpr static float SHOOD_LOAD_FACTOR = 1.85f;
constexpr static float SHOOD_LOAD_FACTOR_INVERSE = SHOOD_LOAD_FACTOR - 1.0f;

template<typename K, typename V>
struct SHoodBucket
{
	K Key;
	V Value;
	uint32_t ProbeLength : 31;
	uint32_t Occupied : 1;
};

template<
	typename K,
	typename V,
	typename HashFunc = DefaultHasher<K>,
	typename EqualsFunc = DefaultEquals<K>>
struct SHoodTable
{
	SHoodBucket<K, V>* Buckets;
	uint32_t Capacity;
	uint32_t Size;
	uint32_t MaxSize;
	SAllocator Allocator;

	void Reserve(uint32_t newCapacity);
	void Clear();
	void Free();

	void Insert(const K* key, const V* val);						// Inserts Key/Value
	SHoodBucket<K, V>* InsertAndGet(const K* key, const V* val);	// Inserts Key/Value returns bucket
	V* InsertKey(const K* key);										// Inserts Key, returns ptr to value
	V* Get(const K* key) const;										// Returns ptr to value
	bool Contains(const K* key) const;
	bool Remove(const K* key);

	inline bool IsAllocated() const { return Buckets; }
	inline size_t Stride() const { return sizeof(SHoodBucket<K, V>); }
	inline size_t MemUsed() const { return Capacity * Stride(); }

	const SHoodBucket<K, V>& operator[](size_t i) const { SASSERT(i < Capacity); return Buckets[i]; }
	SHoodBucket<K, V>& operator[](size_t i) { SASSERT(i < Capacity); return Buckets[i]; }

private:
	uint64_t Hash(const K* key) const;
};

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
uint64_t
SHoodTable<K, V, HashFunc, EqualsFunc>::Hash(const K* key) const
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
void SHoodTable<K, V, HashFunc, EqualsFunc>::Reserve(uint32_t capacity)
{
	uint32_t newCapacity = (uint32_t)((float)capacity * SHOOD_LOAD_FACTOR);
	if (newCapacity == 0)
		newCapacity = 2;
	else if (!IsPowerOf2_32(newCapacity))
		newCapacity = AlignPowTwo32Ceil(newCapacity);

	if (newCapacity <= Capacity) return;

	MaxSize = (uint32_t)((float)newCapacity * SHOOD_LOAD_FACTOR_INVERSE);
	uint32_t oldCapacity = Capacity;
	Capacity = newCapacity;

	if (Size == 0)
	{
		size_t oldSize = oldCapacity * Stride();
		size_t newSize = newCapacity * Stride();
		Buckets = (SHoodBucket<K, V>*)(SRealloc(Allocator, Buckets, oldSize, newSize, MemoryTag::Tables));
	}
	else
	{
		SHoodTable<K, V, HashFunc, EqualsFunc> tmpTable = {};
		tmpTable.Allocator = Allocator;
		tmpTable.Capacity = Capacity;
		tmpTable.MaxSize = MaxSize;
		tmpTable.Buckets = (SHoodBucket<K, V>*)SAlloc(Allocator, newCapacity * Stride(), MemoryTag::Tables);

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
void SHoodTable<K, V, HashFunc, EqualsFunc>::Clear()
{
	SMemClear(Buckets, MemSize());
	Size = 0;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHoodTable<K, V, HashFunc, EqualsFunc>::Free()
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
void SHoodTable<K, V, HashFunc, EqualsFunc>::Insert(const K* key, const V* value)
{
	SASSERT(EqualsFunc{}(key, key));

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	uint64_t hash = Hash(key);

	SHoodBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Value = *value;
	swapBucket.Occupied = true;

	uint64_t index = hash;
	uint32_t probeLength = 0;
	while (true)
	{
		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 1) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, &swapBucket.Key)) return;
			
			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodBucket<K, V> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;
				
				bucket->ProbeLength = probeLength;
			}
			// Continues searching
			++probeLength;
			if (++index == Capacity) index = 0;
		}
		else
		{
			// Note: Found open spot, finish inserting
			*bucket = swapBucket;
			bucket->ProbeLength = probeLength;
			++Size;
			break;
		}
	}
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
SHoodBucket<K, V>* SHoodTable<K, V, HashFunc, EqualsFunc>::InsertAndGet(const K* key, const V* value)
{
	SASSERT(EqualsFunc{}(key, key));

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	uint64_t hash = Hash(key);

	SHoodBucket<K, V>* result = nullptr;

	SHoodBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Value = *value;
	swapBucket.Occupied = true;

	uint64_t index = hash;
	uint32_t probeLength = 0;
	while (true)
	{
		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 1) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, &swapBucket.Key)) return bucket;

			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodBucket<K, V> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;

				bucket->ProbeLength = probeLength;
				probeLength = swapBucket.ProbeLength;

				if (!result)
					result = bucket;
			}
			// Continues searching
			if (++index == Capacity) index = 0;
			++probeLength;
		}
		else
		{
			// Note: Found open spot, finish inserting
			swapBucket.ProbeLength = probeLength;
			*bucket = swapBucket;
			++Size;

			if (!result)
				result = bucket;
			break;
		}
	}
	return result;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
V* SHoodTable<K, V, HashFunc, EqualsFunc>::InsertKey(const K* key)
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(key, key));

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	uint64_t hash = Hash(key);

	V* valuePointer = nullptr;

	SHoodBucket<K, V> swapBucket = {};
	swapBucket.Key = *key;
	swapBucket.Occupied = true;

	uint64_t index = hash;
	uint32_t probeLength = 0;
	while (true)
	{
		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, &swapBucket.Key)) return nullptr;

			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodBucket<K, V> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;

				bucket->ProbeLength = probeLength;
				probeLength = swapBucket.ProbeLength;

				if (!valuePointer)
					valuePointer = &bucket->Value;
			}
			// Continues searching
			if (++index == Capacity) index = 0;
			++probeLength;
		}
		else
		{
			// Note: Found open spot, finish inserting
			swapBucket.ProbeLength = probeLength;
			*bucket = swapBucket;
			++Size;
			bucket->Occupied = true;
			if (!valuePointer)
				valuePointer = &bucket->Value;
			break;
		}

	}
	return valuePointer;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
V* SHoodTable<K, V, HashFunc, EqualsFunc>::Get(const K* key) const
{
	SASSERT(IsAllocated());
	SASSERT(EqualsFunc{}(key, key));
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint32_t index = hash;
	while (true)
	{
		SHoodBucket<K, V>* bucket = &Buckets[index];
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
bool SHoodTable<K, V, HashFunc, EqualsFunc>::Contains(const K* key) const
{
	SASSERT(IsAllocated());
	SASSERT(EqualsFunc{}(key, key));
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint32_t index = hash;
	while (true)
	{
		SHoodBucket<K, V>* bucket = &Buckets[index];
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
bool SHoodTable<K, V, HashFunc, EqualsFunc>::Remove(const K* key)
{
	SASSERT(IsAllocated());
	SASSERT(EqualsFunc{}(key, key));
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint32_t index = hash;
	while (true)
	{
		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 1)
		{
			if (EqualsFunc{}(&bucket->Key, key))
			{
				while (true) // Move any entries after index closer to their ideal probe length.
				{
					uint32_t lastIndex = index;
					if (++index == Capacity) index = 0;

					SHoodBucket<K, V>* nextBucket = &Buckets[index];
					if (nextBucket->Occupied == 0 || nextBucket->ProbeLength == 0) // No more entires to move
					{
						Buckets[lastIndex].ProbeLength = 0;
						Buckets[lastIndex].Occupied = 0;
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
				if (++index == Capacity) index = 0; // continue searching till 0 or found equals key
			}
		}
		else
		{
			break; // No key found
		}
	}
	return false;
}

inline int TestSHoodTable()
{
	SHoodTable<int, int> table = {};
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
