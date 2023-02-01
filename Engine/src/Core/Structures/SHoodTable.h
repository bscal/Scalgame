#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

global_var constexpr float SHOOD_LOAD_FACTOR = 0.75f;
global_var constexpr uint32_t SHOOD_RESIZE = 2;

template<typename K, typename V>
struct SHoodBucket
{
	K Key;
	V Value;
	uint32_t ProbeLength : 31, Occupied : 1;
};

template<typename K, typename V>
struct SHoodTable
{
	SHoodBucket<K, V>* Buckets;
	uint32_t Capacity;
	uint32_t Size;
	uint32_t MaxSize;
	SMemAllocator Allocator;

	void Reserve(uint32_t Capacity);

	void Insert(const K* key, const V* val);
	V* Get(const K* key) const;
	bool Contains(const K* key) const;
	void Remove(const K* key);

	inline bool IsAllocated() const { return (Buckets); }
	inline size_t MemSize() const { return Capacity * sizeof(SHoodBucket<K, V>) };
};

internal inline uint32_t
AlignSize(uint32_t size, uint32_t alignment)
{
    return (size + (alignment - 1)) & -alignment;
}

internal inline bool
IsPowerOf2(uint32_t num)
{
    return (num > 0 && ((num & (num - 1)) == 0));
}

template<typename K>
internal inline uint64_t 
Hash(const K* key, uint32_t capacity)
{
    uint64_t hash = 0;
	hash &= (capacity - 1);
    return hash;
}

template<typename K, typename V>
void SHoodTable<K, V>::Reserve(uint32_t capacity)
{
	Capacity = capacity;
	MaxSize = Capacity * SHOOD_LOAD_FACTOR;
	if (!IsAllocated())
	{
		Buckets = (SHoodBucket<K, V>*)Allocator.Alloc(MemSize());
	}
	else if (Size == 0)
	{
		Allocator.Free(Buckets);
		Buckets = (SHoodBucket<K, V>*)Allocator.Alloc(MemSize());
	}
	else
	{
		SHoodTable<K, V> tmpTable = {};
		tmpTable.Reserve(Capacity);

		for (uint32_t i; i < tmpTable.Capacity; ++i)
		{
			if (Buckets[i].Occupied != 0)
			{
				tmpTable.Insert(Buckets[i].Key, Buckets[i].Value);
			}
			if (tmpTable.Size == Size) break;
		}

		Allocator.Free(Buckets);
		*this = tmpTable;
	}
}

template<typename K, typename V>
void SHoodTable<K, V>::Insert(const K* key, const V* value)
{
    if (Size == MaxSize)
    {
        Reserve(AlignSize(Capacity * SHOOD_RESIZE));
    }

    uint64_t hash = Hash(key, Capacity);

	SHoodBucket<K, V> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Value = *value;
	swapBucket.Occupied = true;

	uint32_t index = hash;
	uint32_t probeLength = 0;
	for (;; ++index; ++probeLength)
	{
		if (index > Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0)
		{
			if (bucket->ProbeLength > probeLength)
			{
				SHoodBucket<K, V> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;
				
				bucket->ProbeLength = probeLength;
				probeLength = swapBucket.ProbeLength;
				++Size;
			}
		}
		else
		{
			swapBucket.ProbeLength = probeLength;
			*bucket = swapBucket;
			break;
		}
	}
}

template<typename K, typename V>
V* SHoodTable<K, V>::Get(const K* key) const
{
	SASSERT(Buckets);
	SASSERT(key);

	uint64_t hash = Hash(key, Capacity);

	uint32_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0)
		{
			if (bucket->Key == *key)
				return &bucket->Value;
		}
		else
			return NULL;

		++index;
	}
	return NULL;
}

template<typename K, typename V>
bool SHoodTable<K, V>::Contains(const K* key) const
{
	SASSERT(Buckets);
	SASSERT(key);

	uint64_t hash = Hash(key, Capacity);

	uint32_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0)
		{
			if (bucket->Key == *key)
				return true
		}
		else
			return false;

		++index;
	}
	return false;
}

template<typename K, typename V>
void SHoodTable<K, V>::Remove(const K* key)
{
	SASSERT(Buckets);
	SASSERT(key);

	uint64_t hash = Hash(key, Capacity);
	uint32_t index = hash;
	while (true)
	{
		if (index == Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied != 0)
		{
			if (bucket->Key == *key)
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

inline void TestSHoodTable()
{
}
