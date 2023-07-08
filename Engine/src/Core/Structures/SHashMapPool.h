#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include "SHashMap.h"

/*
* Stores 
*/
template<typename K, typename V, size_t PoolChunkSize = 4096, typename Hasher = DefaultHasher>
struct SHashMapPool : public SHashMap<K, V*, Hasher>
{
	MemoryPool<V, PoolChunkSize> ValuePool;

	V* InsertPooled(const K* key)
	{
		V* bucket = this->InsertKey(key);
		if (!bucket)
		{
			bucket = ValuePool.allocate();
		}
		return bucket;
	}

	bool RemovePooled(const K* key)
	{
		V* val = nullptr;
		bool wasRemoved = this->RemoveValue(key, &val);
		if (wasRemoved)
		{
			SASSERT(val);
			ValuePool.deallocate(val);
		}
		return wasRemoved;
	}

};
