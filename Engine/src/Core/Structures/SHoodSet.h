#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

global_var constexpr float SSET_LOAD_FACTOR = 0.65f;
global_var constexpr uint32_t SSET_RESIZE = 2;
global_var constexpr uint32_t SSET_DEFAULT_RESERVE_SIZE = 2;

template<typename K>
struct SHoodSetBucket
{
	K Key;
	uint32_t ProbeLength : 31, Occupied : 1;
};

template<typename K,
	typename HashFunc = DefaultHasher<K>,
	typename EqualsFunc = DefaultEquals<K>>
	struct SHoodSet
{
	SMemAllocator Allocator = SMEM_GAME_ALLOCATOR;
	SHoodSetBucket<K>* Buckets;
	uint32_t Size;
	uint32_t Capacity;

	void Reserve(uint32_t capacity);
	void Free();
	void Clear();

	bool Insert(const K* key);
	bool Contains(const K* key) const;
	bool Remove(const K* key);

	bool IsAllocated() const { return MemAllocated() > 0; };
	size_t MemAllocated() const { return Capacity * sizeof(SHoodSetBucket<K>); };
};

template<typename K, typename HashFunc, typename EqualsFunc>
void SHoodSet<K, HashFunc, EqualsFunc>::Reserve(uint32_t capacity)
{
	SASSERT(Allocator.Alloc);
	SASSERT(Allocator.Free);
	if (!IsPowerOf2_32(capacity))
	{
		capacity = AlignPowTwo32Ceil(capacity);
	}
	if (capacity > Capacity)
	{
		uint32_t oldCapacity = Capacity;
		Capacity = capacity;
		if (Size == 0)
		{
			Allocator.Free(Buckets);
			size_t newSize = Capacity * sizeof(K);
			Buckets = static_cast<SHoodSetBucket<K>*>(Allocator.Alloc(newSize * sizeof(SHoodSetBucket<K>)));
		}
		else
		{
			SHoodSet<K, HashFunc, EqualsFunc> tmpSet = {};
			tmpSet.Reserve(Capacity);

			for (uint32_t i = 0; i < oldCapacity; ++i)
			{
				if (Buckets[i].Occupied == 1)
				{
					tmpSet.Insert(&Buckets[i].Key);
				}
				if (tmpSet.Size == Size) break;
			}

			Allocator.Free(Buckets);
			SASSERT(Size == tmpSet.Size);
			*this = tmpSet;
		}
	}

	SASSERT(Buckets);
	SASSERT(Capacity > 0);
	SASSERT(IsPowerOf2_32(Capacity));
}

template<typename K, typename HashFunc, typename EqualsFunc>
void SHoodSet<K, HashFunc, EqualsFunc>::Free()
{
	SASSERT(Allocator);
	SASSERT(Allocator.Alloc);
	SASSERT(Allocator.Free);
	Allocator.Free(Buckets);
	Buckets = NULL;
	Size = 0;
	Capacity = 0;
}

template<typename K, typename HashFunc, typename EqualsFunc>
void SHoodSet<K, HashFunc, EqualsFunc>::Clear()
{
	SMemClear(Buckets, MemAllocated());
	Size = 0;
}

template<typename K, typename HashFunc, typename EqualsFunc>
bool SHoodSet<K, HashFunc, EqualsFunc>::Insert(const K* key)
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(*key, *key))

	if (Capacity == 0) Reserve(SSET_DEFAULT_RESERVE_SIZE);
	else
	{
		float loadFactor = ((float)Size + 1.0f) / (float)Capacity;
		if (loadFactor >= SSET_LOAD_FACTOR)
			Reserve(Capacity * SSET_RESIZE);
	}

	SASSERT(IsAllocated());

	uint64_t hash = HashFunc{}(key);
	uint32_t index = hash & ((uint64_t)(Capacity - 1));
	SASSERT(index < Capacity);

	SHoodSetBucket<K> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Occupied = true;

	uint32_t probeLength = 0;
	while (true)
	{
		if (index == Capacity) index = 0; // Wrap

		SHoodSetBucket<K>* bucket = &Buckets[index];
		if (bucket->Occupied == 1) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(bucket->Key, *key)) return false;

			if (bucket->ProbeLength > probeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodSetBucket<K> tmpBucket = *bucket;

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
			return true;
		}
		++index;
		++probeLength;
	}
	SASSERT_MSG(false, "Container had no room, this should not happen");
	return false;
}


template<typename K, typename HashFunc, typename EqualsFunc>
bool SHoodSet<K, HashFunc, EqualsFunc>::Contains(const K* key) const
{
	SASSERT(key);
	SASSERT(IsAllocated());

	uint64_t hash = HashFunc{}(key);
	uint32_t index = hash & ((uint64_t)(Capacity - 1));
	SASSERT(index < Capacity);
	while (true)
	{
		if (index == Capacity) index = 0; // wraps

		SHoodSetBucket<K, V>* bucket = &Buckets[index];
		if (!bucket->Occupied)
			return false;
		if (EqualsFunc{}(&bucket->Key, key))
			return true;
		++index;
	}
	return false;
}

template<typename K, typename HashFunc, typename EqualsFunc>
bool SHoodSet<K, HashFunc, EqualsFunc>::Remove(const K* key)
{
	SASSERT(key);
	SASSERT(IsAllocated());

	uint64_t hash = HashFunc{}(key);
	uint32_t index = hash & ((uint64_t)(Capacity - 1));
	SASSERT(index < Capacity);
	for (;; ++index)
	{
		if (index == Capacity) index = 0; // wrap

		SHoodBucket<K, V>* bucket = &Buckets[index];
		if (!bucket->Occupied)
			return false; // No key found

		// If equal remove and shift next elements left until probe length is 0
		if (EqualsFunc{}(&bucket->Key, key))
		{
			while (true)
			{
				uint32_t lastIndex = index;
				if (++index == Capacity) index = 0;

				SHoodBucket<K, V>* nextBucket = &Buckets[index];
				if (nextBucket->ProbeLength == 0)
				{
					// We do not need to shift anymore elements because we either
					// found an empty spot or the element is in its ideal spot.
					Buckets[lastIndex].ProbeLength = 0;
					Buckets[lastIndex].Occupied = false;
					--Size;
					return true;
				}
				else
				{
					// Deincrement buckets probe length and copy
					// to last index, then continues
					--nextBucket->ProbeLength;
					Buckets[lastIndex] = *nextBucket;
				}
			}
		}
	}
	return false;
}