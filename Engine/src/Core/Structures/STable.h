#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define S_TABLE_DEFAULT_CAPACITY 10

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

	uint64_t (*KeyHashFunction)(const K* key);
	bool (*KeyEqualsFunction)(const K* v0, const K* v1);
};

template<typename K, typename V>
internal STableEntry<K, V>* CreateEntry(const K* key, const V* value)
{
	STableEntry<K, V>* entry = (STableEntry<K, V>*)
		Scal::MemAllocZero(sizeof(STableEntry<K, V>));
	entry->Key = *key;
	entry->Value = *value;
	return entry;
}

template<typename K, typename V>
internal void FreeEntry(STableEntry<K, V>* entry)
{
	Scal::MemFree(entry);
}

template<typename K, typename V>
internal uint64_t HashKey(STable<K, V>* sTable, const K* key)
{
	uint64_t hash = sTable->KeyHashFunction(key);
	return hash % sTable->Capacity;
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
		capacity = S_TABLE_DEFAULT_CAPACITY;
	}

	// TODO add a way to do this better
	if (!sTable->KeyHashFunction)
	{
		TraceLog(LOG_ERROR, "sTable must have a KeyHashFunction");
		return;
	}
	if (!sTable->KeyEqualsFunction)
	{
		TraceLog(LOG_ERROR, "sTable must have a KeyEqualsFunction");
		return;
	}

	sTable->Size = 0;
	sTable->Capacity = capacity;
	sTable->Entries = (STableEntry<K, V>**)
		Scal::MemAllocZero(sTable->Capacity * sizeof(STableEntry<K, V>*));
}

template<typename K, typename V>
void STableResize(STable<K, V>* sTable, uint64_t newCapacity)
{
	assert(sTable);
	if (newCapacity < sTable->Capacity)
	{
		return;
	}

	sTable->Capacity = newCapacity;
	Scal::MemRealloc(sTable->Entries, sTable->Capacity * sizeof(STableEntry<K, V>*));
}

template<typename K, typename V>
bool STablePut(STable<K, V>* sTable, const K* key, const V* value)
{
	assert(sTable);
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return false;
	}
	if (!value)
	{
		TraceLog(LOG_ERROR, "value cannot be null");
		return false;
	}

	uint64_t hash = HashKey(sTable, key);
	STableEntry<K, V>* entry = sTable->Entries[hash];
	if (!entry) // Empty, insert
	{
		sTable->Entries[hash] = CreateEntry(key, value);
		++sTable->Size;
	}
	else
	{
		STableEntry<K, V>* previous;
		while (entry)
		{
			if (sTable->KeyEqualsFunction(key, &entry->Key))
			{
				// Duplicate, do nothing
				return false;
			}

			previous = entry;
			entry = previous->Next;
		}

		// Collision, add entry to next
		entry = CreateEntry(key, value);
		previous->Next = entry;
		++sTable->Size;
	}
	return true;
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

	uint64_t hash = HashKey(sTable, key);
	STableEntry<K, V>* entry = sTable->Entries[hash];
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

	uint64_t hash = HashKey(sTable, key);
	STableEntry<K, V>* entry = sTable->Entries[hash];
	while (entry)
	{
		if (sTable->KeyEqualsFunction(key, &entry->Key))
			return true;
		entry = entry->Next;
	}
	return false;
}

template<typename K, typename V>
bool STableRemove(STable<K, V>* sTable, const K* key)
{
	assert(sTable);
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return false;
	}

	uint64_t hash = HashKey(sTable, key);
	STableEntry<K, V>* entry = sTable->Entries[hash];
	if (!entry) return false;

	STableEntry<K, V>* previous = entry;
	while (entry->Next)
	{
		if (sTable->KeyEqualsFunction(key, &entry->Key))
		{
			previous->Next = entry->Next;
			FreeEntry(entry);
			--sTable->Size;
			return true;
		}
		previous = entry;
		entry = entry->Next;
	}
	return false;
}

template<typename K, typename V>
void STableDump(STable<K, V>* sTable)
{
	TraceLog(LOG_DEBUG, "Printing STable:");
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
	table.KeyHashFunction = [](const Vector2i* key)
	{
		return (uint64_t)PackVec2i(*key);
	};
	table.KeyEqualsFunction = [](const Vector2i* k0, const Vector2i* k1)
	{
		return *k0 == *k1;
	};

	STableCreate(&table, 10);

	assert(&table);

	Vector2i k = { 2, 2 };
	int v = 8;
	STablePut(&table, &k, &v);

	Vector2i k1 = { 100, 15 };
	int v1 = 4;
	STablePut(&table, &k1, &v1);

	Vector2i k2 = { 2, 2 };
	int v2 = 1;
	STablePut(&table, &k2, &v2);

	int* get = STableGet(&table, &k);
	int value = *get;
	TraceLog(LOG_INFO, "%d", value);

	bool contains = STableContains(&table, &k1);

	STableRemove(&table, &k);

	Vector2i k3 = { 2, 2 };
	int v3= 1;
	STablePut(&table, &k3, &v3);

}
