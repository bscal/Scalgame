#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define STABLE_DEFAULT_CAPACITY 10

global_var constexpr uint64_t STABLE_FULL = UINT64_MAX;

template<typename K, typename V>
struct STableEntry
{
	K Key;
	V Value;
	STableEntry<K, V>* Next;
};

template<typename K, typename V>
struct STable
{
	uint64_t Size;
	uint64_t Capacity;
	STableEntry<K, V>** Entries;
	uint64_t(*KeyHashFunction)(const K* key);
	bool(*KeyEqualsFunction)(const K* v0, const K* v1);
};

template<typename K, typename V>
STableEntry<K, V>* CreateEntry(const K* key, const V* value)
{
	STableEntry<K, V>* entry = (STableEntry<K, V>*)
		Scal::MemAllocZero(sizeof(STableEntry<K, V>));
	entry->Key = *key;
	entry->Value = *value;
	return entry;
}


template<typename K, typename V>
void STableCreate(STable<K, V>* sTable, uint64_t capacity)
{
	if (!sTable)
	{
		TraceLog(LOG_ERROR, "sTable cannot be nullptr");
		return;
	}

	if (capacity < 1)
	{
		capacity = 1;
	}

	sTable->Size = 0;
	sTable->Capacity = capacity;
	sTable->Entries = (STableEntry<K, V>**)
		Scal::MemAllocZero(capacity * sizeof(STableEntry<K, V>*));

	assert(sTable->Entries);
}

template<typename K, typename V>
void STablePut(STable<K, V>* sTable, const K* key, const V* value)
{
	assert(sTable);
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return;
	}
	if (!value)
	{
		TraceLog(LOG_ERROR, "value cannot be null");
		return;
	}

	uint64_t hash = sTable->KeyHashFunction(key);
	hash %= sTable->Capacity;
	STableEntry<K, V>* entry = sTable->Entries[hash];

	if (!entry) // If emptry insert
	{
		sTable->Entries[hash] = CreateEntry(key, value);
	}
	else
	{
		STableEntry<K, V>* previous;
		while (entry)
		{
			if (sTable->KeyEqualsFunction(key, &entry->Key))
			{
				return;
			}

			previous = entry;
			entry = previous->Next;
		}

		entry = CreateEntry(key, value);
		previous->Next = entry;
	}
}

template<typename K, typename V>
V* STableGet(STable<K, V>* sTable, const K* key)
{
	assert(sTable);
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return nullptr;
	}

	uint64_t hash = sTable->KeyHashFunction(key);
	hash %= sTable->Capacity;
	STableEntry<K, V>* entry = sTable->Entries[hash];
	if (!entry)
	{
		return nullptr;
	}

	while (entry)
	{
		if (sTable->KeyEqualsFunction(key, &entry->Key))
		{
			return &entry->Value;
		}
		entry = entry->Next;
	}

	return nullptr;
}

template<typename K, typename V>
bool STableContains(STable<K, V>* sTable, const K* key)
{
	assert(sTable);
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return false;
	}

	uint64_t bucket = 0;
	STableEntry<K, V>* entry = sTable->Entries[bucket];
	if (!entry)
	{
		return false;
	}

	while (entry)
	{
		if (true) // TODO equals
		{
			return true
		}
		entry = entry->Next;
	}

	return false;
}

template<typename K, typename V>
void STableDump(STable<K, V>* sTable)
{
	TraceLog(LOG_DEBUG, LOG_DEBUG"Printing STable:");
	for (int i = 0; i < sTable->Size; ++i)
	{
		STableEntry<K, V>* entry = sTable->Entries[i];
		if (entry)
		{
			TraceLog(LOG_DEBUG, "* Bucket(%d)", i);
			for (;;)
			{
				TraceLog(LOG_DEBUG, "%s=%s", entry->Key, entry->Value);
				if (entry->Next)
					entry = entry->Next;
				else
					break;
			}
		}
	}
	TraceLog(LOG_DEBUG, "");
}

inline void TestSTable()
{
	STable<Vector2i, int> table = {};
	STableCreate(&table, 10);

	table.KeyHashFunction = [](const Vector2i* key)
	{
		return (uint64_t)PackVec2i(*key);
	};
	table.KeyEqualsFunction = [](const Vector2i* k0, const Vector2i* k1)
	{
		return *k0 == *k1;
	};

	Vector2i k = { 2, 2 };
	int v = 8;
	STablePut(&table, &k, &v);

	int* get = STableGet(&table, &k);
	int value = *get;
	TraceLog(LOG_INFO, "%d", value);
}
