#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define S_TABLE_DEFAULT_CAPACITY 10
#define S_TABLE_DEFAULT_SPACE = 0.75f
#define S_TABLE_DEFAULT_RESIZE = 2

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
		Scal::MemAlloc(sizeof(STableEntry<K, V>));
	entry->Key = *key;
	entry->Value = *value;
	entry->Next = 0;
	return entry;
}

template<typename K, typename V>
internal void FreeEntry(STableEntry<K, V>* entry)
{
	Scal::MemFree(entry);
	entry = 0;
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
internal STable<K, V> Rehash(STable<K, V>* sTable);

template<typename K, typename V>
void STableResize(STable<K, V>* sTable, uint64_t newCapacity)
{
	assert(sTable);
	if (newCapacity < sTable->Capacity)
	{
		newCapacity = sTable->Capacity * 2;
	}
	sTable->Capacity = newCapacity;
	*sTable = Rehash(sTable);
	TraceLog(LOG_INFO, "Resizing!");
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

	if ((float)sTable->Size >= (float)sTable->Capacity * 0.75f)
	{
		STableResize(sTable, sTable->Capacity * 2);
	}

	uint64_t hash = HashKey(sTable, key);
	STableEntry<K, V>* entry = sTable->Entries[hash];
	if (!entry)
	{
		sTable->Entries[hash] = CreateEntry(key, value);
		++sTable->Size;
		return true;
	}

	STableEntry<K, V>* previous = entry;
	while (entry)
	{
		if (sTable->KeyEqualsFunction(key, &entry->Key))
		{
			return false;
		}

		previous = entry;
		entry = previous->Next;
	}

	// Collision, add entry to next
	entry = CreateEntry(key, value);
	previous->Next = entry;
	++sTable->Size;
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
	if (!entry->Next)
	{
		FreeEntry(entry);
		sTable->Entries[hash] = 0;
		--sTable->Size;
		return true;
	}

	STableEntry<K, V>* previous = 0;
	while (entry->Next)
	{
		if (sTable->KeyEqualsFunction(key, &entry->Key))
		{
			FreeEntry(entry);

			if (previous)
				previous->Next = entry->Next;
			else
				sTable->Entries[hash] = entry->Next;

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

template<typename K, typename V>
internal STable<K, V> Rehash(STable<K, V>* sTable)
{
	STable<K, V> sTable2 = *sTable;
	STableCreate(&sTable2, sTable->Capacity);

	for (int i = 0; i < sTable->Capacity; ++i)
	{
		STableEntry<K, V>* entry = sTable->Entries[i];
		if (entry)
		{
			while (entry->Next)
			{
				auto next = entry->Next;
				STablePut(&sTable2, &next->Key, &next->Value);
				STableRemove(sTable, &next->Key);
			}
			STablePut(&sTable2, &entry->Key, &entry->Value);
			STableRemove(sTable, &entry->Key);
		}
	}

	Scal::MemFree(sTable->Entries);
	sTable->Entries = 0;
	assert(sTable->Entries == nullptr);
	assert(sTable2.Entries != nullptr);
	return sTable2;
}


inline void TestSTable()
{
	STable<Vector2i, char> table = {};
	table.KeyHashFunction = [](const Vector2i* key)
	{
		return (uint64_t)PackVec2i(*key);
	};
	table.KeyEqualsFunction = [](const Vector2i* k0, const Vector2i* k1)
	{
		return *k0 == *k1;
	};
	STableCreate(&table, 5);

	assert(table.Entries);

	Vector2i k = { 1, 2 };
	char v = 1;
	STablePut(&table, &k, &v);

	Vector2i kk = { 2, 2 };
	char vv = 2;
	STablePut(&table, &kk, &vv);

	Vector2i kk1 = { 3, 2 };
	char vv1 = 3;
	STablePut(&table, &kk1, &vv1);

	Vector2i kk2 = { 4, 2 };
	char vv2 = 4;
	STablePut(&table, &kk2, &vv2);

	Vector2i kk3 = { 5, 2 };
	char vv3 = 5;
	STablePut(&table, &kk3, &vv3);

	Vector2i kk4 = { 6, 2 };
	char vv4 = 6;
	STablePut(&table, &kk4, &vv4);

	assert(table.Size == 6);

	Vector2i k1 = { 100, 15 };
	char v1 = 255;
	STablePut(&table, &k1, &v1);

	assert(table.Size == 7);

	Vector2i k2 = { 2, 2 };
	char v2 = 64;
	STablePut(&table, &k2, &v2);

	assert(table.Size == 7);

	char* get = STableGet(&table, &k);
	assert(*get == v);

	bool contains = STableContains(&table, &k1);

	assert(contains == true);

	bool removed = STableRemove(&table, &kk);

	assert(removed == true);

	Vector2i k3 = { 2, 2 };
	char v3 = 128;
	bool added = STablePut(&table, &k3, &v3);

	assert(added == true);
}
