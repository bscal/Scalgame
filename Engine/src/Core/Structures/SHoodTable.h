#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

global_var constexpr float SHOOD_LOAD_FACTOR = 0.75f;
global_var constexpr uint32_t SHOOD_RESIZE = 2;

template<typename K>
struct SHoodDefaultHash
{
	[[nodiscard]] inline uint64_t operator()(const K* key) const
	{
		const uint8_t* const data = (const uint8_t * const)key;
		return FNVHash64(data, sizeof(K));
	}
};

template<typename K>
struct SHoodDefaultEquals
{
	[[nodiscard]] inline bool operator()(const K* k1, const K* k2) const
	{
		return *k1 == *k2;
	}
};


template<typename K, typename V>
struct SHoodBucket
{
	K Key;
	V Value;
	uint32_t ProbeLength : 31, Occupied : 1;
};

template<
	typename K,
	typename V,
	typename HashFunc = SHoodDefaultHash<K>,
	typename EqualsFunc = SHoodDefaultEquals<K>>
struct SHoodTable
{
	SMemAllocator Allocator = SMEM_GAME_ALLOCATOR;
	SHoodBucket<K, V>* Buckets;
	uint32_t Capacity;
	uint32_t Size;
	uint32_t MaxSize;

	void Reserve(uint32_t newCapacity);
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
	if (capacity == 0) capacity = 2;
	else if (!IsPowerOf2_32(capacity))
	{
		capacity = AlignPowTwo32Ceil(capacity);
		SLOG_WARN("SHoodTable::Reserve capacity is not a power of 2, automatically aligning");
	}

	SASSERT(capacity > Capacity);

	Capacity = capacity;
	MaxSize = (uint32_t)((float)Capacity * SHOOD_LOAD_FACTOR);
	if (!IsAllocated())
	{
		Buckets = (SHoodBucket<K, V>*)Allocator.Alloc(MemSize());
		SMemClear(Buckets, MemSize());
	}
	else if (Size == 0)
	{
		Allocator.Free(Buckets);
		Buckets = (SHoodBucket<K, V>*)Allocator.Alloc(MemSize());
		SMemClear(Buckets, MemSize());
	}
	else
	{
		SHoodTable<K, V, HashFunc, EqualsFunc> tmpTable = {};
		tmpTable.Reserve(Capacity);

		for (uint32_t i = 0; i < tmpTable.Capacity; ++i)
		{
			if (Buckets[i].Occupied != 0)
			{
				tmpTable.Insert(&Buckets[i].Key, &Buckets[i].Value);
			}
			if (tmpTable.Size == Size) break;
		}

		Allocator.Free(Buckets);
		*this = tmpTable;
	}
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHoodTable<K, V, HashFunc, EqualsFunc>::Free()
{
	Allocator.Free(Buckets);
	SMemClear(this, sizeof(SHoodTable<K, V, HashFunc, EqualsFunc>));
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
void SHoodTable<K, V, HashFunc, EqualsFunc>::Insert(const K* key, const V* value)
{
    if (Size >= MaxSize)
    {
        Reserve(Capacity * SHOOD_RESIZE);
    }

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
		if (bucket->Occupied != 0) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, key)) return;
			
			if (bucket->ProbeLength > probeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodBucket<K, V> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;
				
				bucket->ProbeLength = probeLength;
				probeLength = swapBucket.ProbeLength;
			}

			// Continues searching
		}
		else
		{
			// Note: Found open spot, finish inserting
			swapBucket.ProbeLength = probeLength;
			*bucket = swapBucket;
			++Size;
			break;
		}
		++index;
		++probeLength;
	}
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
V* SHoodTable<K, V, HashFunc, EqualsFunc>::InsertKey(const K* key)
{
	if (Size >= MaxSize)
	{
		Reserve(Capacity * SHOOD_RESIZE);
	}

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
			if (EqualsFunc{}(&bucket->Key, key)) return NULL;

			if (bucket->ProbeLength > probeLength)
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
		++index;
		++probeLength;
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
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint64_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0)
		{
			if (EqualsFunc{}(&bucket->Key, key))
				return &bucket->Value;
		}
		else
			return NULL;

		++index;
	}
	return NULL;
}

template<
	typename K,
	typename V,
	typename HashFunc,
	typename EqualsFunc>
bool SHoodTable<K, V, HashFunc, EqualsFunc>::Contains(const K* key) const
{
	SASSERT(IsAllocated());
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint64_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0 &&
			EqualsFunc{}(&bucket->Key, key))
		{
			return true;
		}
		else
			return false;

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
	SASSERT(key);

	uint64_t hash = Hash(key);
	uint64_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0)
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
						--nextBucket->ProbeLength;
						Buckets[lastIndex] = *nextBucket;
					}
					else
					{
						Buckets[lastIndex].Occupied = false;
						--Size;
						return;
					}
				}
			}
		}
		else
		{
			return; // No key found
		}

		++index;
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
	SASSERT(table.MaxSize == 1);
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
	SASSERT(table.MaxSize == 32 * SHOOD_LOAD_FACTOR);
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
	SASSERT(*ikvget == 101)

	table.Free();
	SASSERT(!table.Buckets);
	SASSERT(table.Capacity == 0);

	return 1;
}
