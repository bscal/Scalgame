#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

global_var constexpr float SHOOD_LOAD_FACTOR = 0.60f;
global_var constexpr float SHOOD_LOAD_MULTIPLIER = 1.0f + SHOOD_LOAD_FACTOR;
global_var constexpr uint32_t SHOOD_RESIZE = 2;

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
	SMemAllocator Allocator = SMEM_GAME_ALLOCATOR;
	SHoodBucket<K, V>* Buckets;
	uint32_t Capacity;
	uint32_t Size;
	uint32_t MaxSize;

	void Reserve(uint32_t newCapacity);
	void Clear();
	void Free();

	void Insert(const K* key, const V* val);
	V* InsertKey(const K* key);
	V* Get(const K* key) const;
	bool Contains(const K* key) const;
	void Remove(const K* key);

	inline bool IsAllocated() const { return (Buckets); }
	inline size_t MemSize() const { return Capacity * sizeof(SHoodBucket<K, V>); };

private:
	inline uint64_t Hash(const K* key) const;
};

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
inline uint64_t 
SHoodTable<K, V, HashFunc, EqualsFunc>::Hash(const K* key) const
{
	SASSERT(IsPowerOf2_32(Capacity));
	uint64_t hash = HashFunc{}(key);
	hash &= ((uint64_t)(Capacity - 1));
    return hash;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHoodTable<K, V, HashFunc, EqualsFunc>::Reserve(uint32_t capacity)
{
	SASSERT(Allocator.Alloc);
	SASSERT(Allocator.Free);

	uint32_t newCapacity = (uint32_t)((float)capacity * SHOOD_LOAD_MULTIPLIER);
	if (newCapacity == 0)
		newCapacity = 2;
	else if (!IsPowerOf2_32(newCapacity))
		newCapacity = AlignPowTwo32Ceil(newCapacity);

	if (newCapacity <= Capacity) return;

	uint32_t oldCapacity = Capacity;
	Capacity = newCapacity;
	MaxSize = (uint32_t)((float)Capacity * SHOOD_LOAD_FACTOR);

	if (Size == 0)
	{
		Allocator.Free(Buckets);
		size_t newSize = MemSize();
		Buckets = (SHoodBucket<K, V>*)(Allocator.Alloc(newSize));
	}
	else
	{
		SHoodTable<K, V, HashFunc, EqualsFunc> tmpTable = {};
		tmpTable.Allocator = Allocator;
		tmpTable.Capacity = Capacity;
		tmpTable.MaxSize = MaxSize;
		tmpTable.Buckets = (SHoodBucket<K, V>*)(Allocator.Alloc(MemSize()));

		for (uint32_t i = 0; i < oldCapacity; ++i)
		{
			if (Buckets[i].Occupied == 1)
			{
				tmpTable.Insert(&Buckets[i].Key, &Buckets[i].Value);
			}
			if (tmpTable.Size == Size) break;
		}

		Allocator.Free(Buckets);
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
	Allocator.Free(Buckets);
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
		if (index == Capacity) index = 0; // Wrap

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 1) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, key)) return;
			
			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodBucket<K, V> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;
				
				bucket->ProbeLength = probeLength;
				probeLength = swapBucket.ProbeLength;
			}
			// Continues searching
			++index;
			++probeLength;
		}
		else
		{
			// Note: Found open spot, finish inserting
			swapBucket.ProbeLength = probeLength;
			*bucket = swapBucket;
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
V* SHoodTable<K, V, HashFunc, EqualsFunc>::InsertKey(const K* key)
{
	SASSERT(EqualsFunc{}(key, key));

	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

	SASSERT(Buckets);

	uint64_t hash = Hash(key);

	V* valuePointer = NULL;

	SHoodBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Occupied = true;

	uint64_t index = hash;
	uint32_t probeLength = 0;
	while (true)
	{
		if (index == Capacity) index = 0; // Wrap

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, key)) return nullptr;

			if (probeLength > bucket->ProbeLength)
			{
				if (!valuePointer) valuePointer = &bucket->Value;

				// Note: Swap out current insert with bucket
				SHoodBucket<K, V> tmpBucket = *bucket;

				swapBucket.ProbeLength = probeLength;
				*bucket = swapBucket;

				swapBucket = tmpBucket;
				probeLength = swapBucket.ProbeLength;
			}
			// Continues searching
			++index;
			++probeLength;
		}
		else
		{
			// Note: Found open spot, finish inserting
			swapBucket.ProbeLength = probeLength;
			*bucket = swapBucket;
			++Size;
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
	uint64_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 0)
			return nullptr;
		else if (EqualsFunc{}(&bucket->Key, key))
			return &bucket->Value;
		else
			++index;
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
	uint64_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 0)
			return false;
		else if (EqualsFunc{}(&bucket->Key, key))
			return true;
		else
			++index;
	}
	return false;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHoodTable<K, V, HashFunc, EqualsFunc>::Remove(const K* key)
{
	SASSERT(IsAllocated());
	SASSERT(EqualsFunc{}(key, key));
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint64_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied == 1)
		{
			if (EqualsFunc{}(&bucket->Key, key))
			{
				uint32_t lastIndex = index;
				while (true)
				{
					if (++index == Capacity) index = 0;

					SHoodBucket<K, V>* nextBucket = &Buckets[index];
					if (nextBucket->ProbeLength != 0)
					{
						Buckets[lastIndex] = *nextBucket;
						Buckets[lastIndex].ProbeLength--;
					}
					else
					{
						Buckets[lastIndex].ProbeLength = 0;
						Buckets[lastIndex].Occupied = 0;
						--Size;
						return;
					}
				}
			}
			++index; // continue searching till 0 or found equals key
		}
		else
		{
			break; // No key found
		}
	}
}

inline int TestSHoodTable()
{
	SHoodTable<int, int> table = {};
	SASSERT(!table.IsAllocated());

	int k0 = 5;
	int v1 = 255;
	table.Insert(&k0, &v1);

	SASSERT(table.Size == 1);
	SASSERT(table.MaxSize == uint32_t((float)table.Capacity * SHOOD_LOAD_FACTOR));
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
	SASSERT(table.MaxSize == uint32_t((float)table.Capacity * SHOOD_LOAD_FACTOR));
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
