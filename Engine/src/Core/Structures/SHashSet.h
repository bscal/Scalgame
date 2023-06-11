#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

constexpr static uint32_t SSET_RESIZE = 2u;
constexpr static float SSET_LOAD_FACTOR = 1.80f;
constexpr static float SSET_LOAD_FACTOR_INVERSE = SSET_LOAD_FACTOR - 1.0f;

template<typename K>
struct SHoodSetBucket
{
	K Key;
	uint32_t ProbeLength : 31;
	uint32_t Occupied : 1;
};

// Hashset using Robin Hood open addressing
template<typename K, typename HashFunc = DefaultHasher<K>, typename EqualsFunc = DefaultEquals<K>>
struct SHashSet
{
	SHoodSetBucket<K>* Buckets;
	uint32_t Size;
	uint32_t Capacity;
	uint32_t MaxSize;
	SAllocator Allocator;

	void Reserve(uint32_t capacity);
	void Free();
	void Clear();

	bool Insert(const K* key);
	bool Contains(const K* key) const;
	bool Remove(const K* key);

	_FORCE_INLINE_ bool IsAllocated() const { return MemUsed() > 0; };
	_FORCE_INLINE_ size_t Stride() const { return sizeof(SHoodSetBucket<K>); }
	_FORCE_INLINE_ size_t MemUsed() const { return Capacity * Stride(); };

	void ToString() const;
};

template<typename K, typename HashFunc, typename EqualsFunc>
void SHashSet<K, HashFunc, EqualsFunc>::Reserve(uint32_t capacity)
{
	uint32_t newCapacity = (uint32_t)((float)capacity * SSET_LOAD_FACTOR);
	if (newCapacity == 0)
		newCapacity = 2;
	else if (!IsPowerOf2_32(newCapacity))
		newCapacity = AlignPowTwo32Ceil(newCapacity);

	if (newCapacity <= Capacity) return;

	if (Size == 0)
	{
		size_t oldSize = Capacity * Stride();
		size_t newSize = newCapacity * Stride();
		Buckets = (SHoodSetBucket<K>*)(SRealloc(Allocator, Buckets, oldSize, newSize, MemoryTag::Tables));
		Capacity = newCapacity;
		MaxSize = (uint32_t)((float)newCapacity * SSET_LOAD_FACTOR_INVERSE);
	}
	else
	{
		SHashSet<K, HashFunc, EqualsFunc> tmpSet = {};
		tmpSet.Allocator = Allocator;
		tmpSet.Size = 0;
		tmpSet.Capacity = newCapacity;
		tmpSet.MaxSize = (uint32_t)((float)newCapacity * SSET_LOAD_FACTOR_INVERSE);
		tmpSet.Buckets = (SHoodSetBucket<K>*)(SAlloc(Allocator, tmpSet.Capacity * Stride(), MemoryTag::Tables));

		for (uint32_t i = 0; i < Capacity; ++i)
		{
			if (Buckets[i].Occupied)
			{
				bool b = tmpSet.Insert(&Buckets[i].Key);
			}
		}

		SASSERT(Size == tmpSet.Size);
		SFree(Allocator, Buckets, MemUsed(), MemoryTag::Tables);
		*this = tmpSet;
	}

	SASSERT(Buckets);
	SASSERT(IsAllocated());
	SASSERT(IsPowerOf2_32(Capacity));
}

template<typename K, typename HashFunc, typename EqualsFunc>
void SHashSet<K, HashFunc, EqualsFunc>::Free()
{
	SFree(Allocator, Buckets, MemUsed(), MemoryTag::Tables);
	Buckets = nullptr;
	Size = 0;
	Capacity = 0;
	MaxSize = 0;
}

template<typename K, typename HashFunc, typename EqualsFunc>
void SHashSet<K, HashFunc, EqualsFunc>::Clear()
{
	if (Buckets)
		SMemClear(Buckets, MemUsed());
	Size = 0;
}

template<typename K, typename HashFunc, typename EqualsFunc>
bool SHashSet<K, HashFunc, EqualsFunc>::Insert(const K* key)
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(key, key))

	if (Size >= MaxSize)
		Reserve(Capacity * SSET_RESIZE);

	SASSERT(IsAllocated());

	SHoodSetBucket<K> swapBucket;
	swapBucket.Key = *key;
	swapBucket.Occupied = true;

	uint64_t hash = HashFunc{}(key);
	uint32_t index = hash & ((uint64_t)(Capacity - 1));
	SASSERT(index < Capacity);
	uint32_t probeLength = 0;
	while (true)
	{
		SHoodSetBucket<K>* bucket = &Buckets[index];
		if (bucket->Occupied) // Bucket is being used
		{
			// Duplicate
			if (EqualsFunc{}(&bucket->Key, &swapBucket.Key))
				return false;

			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				SHoodSetBucket<K> tmpBucket = *bucket;
				*bucket = swapBucket;
				swapBucket = tmpBucket;

				bucket->ProbeLength = probeLength;
			}
			// Continues searching
			++probeLength;
			if (++index == Capacity)
				index = 0;
		}
		else
		{
			// Note: Found open spot, finish inserting
			*bucket = swapBucket;
			bucket->ProbeLength = probeLength;
			++Size;
			return true;
		}
	}
	SASSERT_MSG(false, "Container had no room, this should not happen");
	return false;
}


template<typename K, typename HashFunc, typename EqualsFunc>
bool SHashSet<K, HashFunc, EqualsFunc>::Contains(const K* key) const
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(key, key));

	if (Size == 0) return false;

	uint64_t hash = HashFunc{}(key);
	uint32_t index = hash & ((uint64_t)(Capacity - 1));
	SASSERT(index < Capacity);
	while (true)
	{
		SHoodSetBucket<K>* bucket = &Buckets[index];
		if (!bucket->Occupied)
			return false;
		if (EqualsFunc{}(&bucket->Key, key))
			return true;
		else
			if (index++ == Capacity) index = 0; // wraps
	}
	return false;
}

template<typename K, typename HashFunc, typename EqualsFunc>
bool SHashSet<K, HashFunc, EqualsFunc>::Remove(const K* key)
{
	SASSERT(key);
	SASSERT(EqualsFunc{}(key, key))
	SASSERT(IsAllocated());

	uint64_t hash = HashFunc{}(key);
	uint32_t index = hash & ((uint64_t)(Capacity - 1));
	SASSERT(index < Capacity);
	while(true)
	{
		SHashMapBucket<K>* bucket = &Buckets[index];
		if (!bucket->Occupied)
			return false; // No key found

		// If equal remove and shift next elements left until probe length is 0
		if (EqualsFunc{}(&bucket->Key, key))
		{
			while (true)
			{
				uint32_t lastIndex = index;
				if (++index == Capacity) index = 0;

				SHashMapBucket<K, V>* nextBucket = &Buckets[index];
				if (nextBucket->ProbeLength == 0 || nextBucket->Occupied == 0)
				{
					// We do not need to shift anymore elements because we either
					// found an empty spot or the element is in its ideal spot.
					Buckets[lastIndex].ProbeLength = 0;
					Buckets[lastIndex].Occupied = 0;
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
		else
		{
			if (++index == Capacity) index = 0;
		}
	}
	return false;
}

template<typename K, typename HashFunc, typename EqualsFunc>
void SHashSet<K, HashFunc, EqualsFunc>::ToString() const
{
	SLOG_INFO("\nSHoodSet Info: Size:%u/MaxSize:%u , Capacity: %u", Size, MaxSize, Capacity);
	for (uint32_t i = 0; i < Capacity; ++i)
	{
		const SHoodSetBucket<K>& bucket = Buckets[i];
		SLOG_INFO("%d = probe:%d, occupied:%d, hash:%u", i, bucket.ProbeLength, bucket.Occupied, HashFunc{}(&bucket.Key) % Capacity);
	}
}