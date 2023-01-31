#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

global_var constexpr float SHOOD_LOAD_FACTOR = 0.8f;

template<typename K, typename V>
struct SHoodBucket
{
	K Key;
	V Value;
	uint32_t Length : 31, Occupied : 1;
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
	void Remove(const K* key);
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
void SHoodTable<K, V>::Insert(const K* key, const V* val)
{
    if (Size == MaxSize)
    {
        return;
    }

    uint64_t hash = Hash(key, Capacity);

	SHoodBucket<K, V> swapBucket;
	uint32_t index = hash;
	uint32_t probeLength = 0;
	bool probing = true;
	while (probing)
	{
		if (index > Capacity) index = 0;

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (bucket->Occupied)
		{
			if (bucket->Length > probeLength)
			{
				swapBucket = bucket;
				bucket->Key = *key;
				bucket->Value = *val;
				bucket->Length = probeLength;
				probeLength = swapBucket.Length;
			}
		}
		else
		{
			bucket->Key = *key;
			bucket->Value = *value;
			bucket->Length = probeLength;
			bucket->Occupied = true;
			probing = false;
		}
		++index;
		++probeLength;
	}
}
