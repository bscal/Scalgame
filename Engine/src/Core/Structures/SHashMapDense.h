#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include "SHashMap.h"

/*
* Stores 
*/
template<typename K, typename V, typename Hasher = DefaultHasher>
struct SHashHapDense : public SHashMap<K, V*, Hasher>
{
	V* InsertDense(const K* key)
	{
		V* valuePtr = SAlloc(Allocator, sizeof(V), MemoryTag::Game);
		SASSERT(valuePtr);

		uint32_t insertIdx this->Insert(key, valuePtr);

		return valuePtr;
	}

	bool RemoveDense(const K* key)
	{
		V* val = nullptr;
		bool wasRemoved = this->RemoveValue(key, &val);
		if (wasRemoved)
		{
			SASSERT(val);
			SFree(Allocator, val, sizeof(V), MemoryTag::Game);
		}
		return wasRemoved;
	}
};
