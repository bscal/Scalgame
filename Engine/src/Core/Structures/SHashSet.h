#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

constexpr static uint32_t HASHSET_RESIZE = 2u;
constexpr static float HASHSET_LOAD_FACTOR = 1.80f;
constexpr static float SSET_LOAD_FACTOR_INVERSE = HASHSET_LOAD_FACTOR - 1.0f;

template<typename K>
struct SHashSetBucket
{
	K Key;
	uint32_t ProbeLength : 31;
	uint32_t Occupied : 1;
};

// Hashset using Robin Hood open addressing
template<typename K, typename HashFunc = DefaultHasher>
struct SHashSet
{
	SHashSetBucket<K>* Buckets;
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

	_FORCE_INLINE_ bool IsAllocated() const { return Buckets; };

	void ToString() const;

private:
	_FORCE_INLINE_ uint32_t Hash(const K* key) const
	{
		SASSERT(IsPowerOf2_32(Capacity));
		// Fast power of 2 hash
		uint32_t hash = HashFunc{}(key, sizeof(K));
		hash &= (Capacity - 1);
		SASSERT(hash < Capacity);
		return hash;
	}
};

template<typename K, typename HashFunc>
void SHashSet<K, HashFunc>::Reserve(uint32_t capacity)
{
	constexpr size_t stride = sizeof(SHashSetBucket<K>);

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
		Buckets = (SHashSetBucket<K>*)SRealloc(Allocator, Buckets, oldSize, newSize, MemoryTag::Tables);
	}
	else
	{
		SHashSet<K> tmpTable;
		tmpTable.Capacity = Capacity;
		tmpTable.MaxSize = MaxSize;
		tmpTable.Size = 0;
		tmpTable.Allocator = Allocator;
		tmpTable.Buckets = (SHashSetBucket<K>*)SAlloc(tmpTable.Allocator, newSize, MemoryTag::Tables);

		for (uint32_t i = 0; i < oldCapacity; ++i)
		{
			if (Buckets[i].Occupied)
				tmpTable.Insert(&Buckets[i].Key);

			if (tmpTable.Size == Size)
				break;
		}

		SFree(Allocator, Buckets, oldSize, MemoryTag::Tables);

		SASSERT(Size == tmpTable.Size);
		SLOG_DEBUG("SHashMap resized! From: %u, To: %u", oldCapacity, Capacity);

		*this = tmpTable;
	}

	SASSERT(Buckets);
	SASSERT(Capacity > 0);
	SASSERT(IsPowerOf2_32(Capacity));
}

template<typename K, typename HashFunc>
void SHashSet<K, HashFunc>::Free()
{
	size_t size = Capacity * sizeof(SHashSetBucket<K>);
	SFree(Allocator, Buckets, size, MemoryTag::Tables);
	SMemClear(this, sizeof(SHashSet<K, HashFunc>));
}

template<typename K, typename HashFunc>
void SHashSet<K, HashFunc>::Clear()
{
	if (Buckets)
	{
		size_t size = Capacity * sizeof(SHashSetBucket<K>);
		SMemClear(Buckets, size);
	}
	Size = 0;
}

template<typename K, typename HashFunc>
bool SHashSet<K, HashFunc>::Insert(const K* key)
{
	SASSERT(key);

	if (Size >= MaxSize)
		Reserve(Capacity * HASHSET_RESIZE);

	SASSERT(Buckets);

	SHashSetBucket<K> swapBucket;
	swapBucket.Key = *key;
	swapBucket.ProbeLength = 0;
	swapBucket.Occupied = true;

	uint32_t index = Hash(key);
	uint32_t probeLength = 0;
	while (true)
	{
		SHashSetBucket<K>* bucket = &Buckets[index];
		if (bucket->Occupied) // Bucket is being used
		{
			// Duplicate
			if (bucket->Key == swapBucket.Key)
				return false;

			if (probeLength > bucket->ProbeLength)
			{
				// Note: Swap out current insert with bucket
				swapBucket.ProbeLength = probeLength;
				probeLength = bucket->ProbeLength;
				Swap(*bucket, swapBucket, SHashSetBucket<K>);
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

template<typename K, typename HashFunc>
bool SHashSet<K, HashFunc>::Contains(const K* key) const
{
	SASSERT(key);

	if (Size == 0)
		return false;

	uint32_t index = Hash(key);
	while (true)
	{
		SHashSetBucket<K>* bucket = &Buckets[index];
		if (!bucket->Occupied)
			return false;
		if (*key == bucket->Key)
			return true;
		else
			if (index++ == Capacity) index = 0; // wraps
	}
	return false;
}

template<typename K, typename HashFunc>
bool SHashSet<K, HashFunc>::Remove(const K* key)
{
	SASSERT(key);
	SASSERT(IsAllocated());

	uint32_t index = Hash(key);
	while (true)
	{
		SHashSetBucket<K>* bucket = &Buckets[index];
		if (!bucket->Occupied)
			return false; // No key found

		if (*key == bucket->Key)
		{
			while (true) // Move any entries after index closer to their ideal probe length.
			{
				uint32_t lastIndex = index;
				if (++index == Capacity)
					index = 0;

				SHashSetBucket<K>* nextBucket = &Buckets[index];
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
				index = 0;
		}
	}
	return false;
}

template<typename K, typename HashFunc>
void SHashSet<K, HashFunc>::ToString() const
{
	SLOG_INFO("\nSHoodSet Info: Size:%u/MaxSize:%u , Capacity: %u", Size, MaxSize, Capacity);
	for (uint32_t i = 0; i < Capacity; ++i)
	{
		const SHashSetBucket<K>& bucket = Buckets[i];
		SLOG_INFO("%d = probe:%d, occupied:%d, hash:%u", i, bucket.ProbeLength, bucket.Occupied, HashFunc{}(&bucket.Key, sizeof(K)) % Capacity);
	}
}
