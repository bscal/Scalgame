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

	uint64_t(*KeyHashFunction)(const K* key);
	bool (*KeyEqualsFunction)(const K* v0, const K* v1);

	void Initialize(uint64_t capacity);
	void Resize(uint64_t newCapacity);
	bool Put(const K* key, V* value);
	V* Get(const K* key);
	bool Remove(const K* key);
	bool Contains(const K* key) const;
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
internal uint64_t HashKey(const STable<K, V>* sTable, const K* key)
{
	uint64_t hash = sTable->KeyHashFunction(key);
	return hash % sTable->Capacity;
}

template<typename K, typename V>
internal STable<K, V> Rehash(STable<K, V>* sTable)
{
	STable<K, V> sTable2 = *sTable;
	sTable2.Initialize(sTable->Capacity);

	for (int i = 0; i < sTable->Capacity; ++i)
	{
		STableEntry<K, V>* entry = sTable->Entries[i];
		if (entry)
		{
			while (entry->Next)
			{
				auto next = entry->Next;
				sTable2.Put(&next->Key, &next->Value);
				sTable->Remove(&next->Key);
			}
			sTable2.Put(&entry->Key, &entry->Value);
			sTable->Remove(&entry->Key);
		}
	}

	Scal::MemFree(sTable->Entries);

	*sTable = sTable2;
	return *sTable;
}

template<typename K, typename V>
void STable<typename K, typename V>::Initialize(uint64_t capacity)
{
	if (capacity < 1)
	{
		capacity = S_TABLE_DEFAULT_CAPACITY;
	}

	// TODO add a way to do this better
	if (!KeyHashFunction)
	{
		TraceLog(LOG_ERROR, "sTable must have a KeyHashFunction");
		return;
	}
	if (!KeyEqualsFunction)
	{
		TraceLog(LOG_ERROR, "sTable must have a KeyEqualsFunction");
		return;
	}

	Size = 0;
	Capacity = capacity;
	Entries = (STableEntry<K, V>**)Scal::MemAllocZero(Capacity * sizeof(STableEntry<K, V>*));
}

template<typename K, typename V>
void STable<typename K, typename V>::Resize(uint64_t newCapacity)
{
	if (newCapacity < Capacity)
	{
		newCapacity = Capacity * 2;
	}
	Capacity = newCapacity;
	Rehash(this);
	TraceLog(LOG_INFO, "Resizing!");
}

template<typename K, typename V>
bool STable<typename K, typename V>::Put(const K* key, V* value)
{
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

	if ((float)Size >= (float)Capacity * 0.75f)
	{
		Resize(Capacity * 2);
	}

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	if (!entry)
	{
		Entries[hash] = CreateEntry(key, value);
		++Size;
		return true;
	}

	STableEntry<K, V>* previous = entry;
	while (entry)
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			return false;
		}

		previous = entry;
		entry = previous->Next;
	}

	// Collision, add entry to next
	entry = CreateEntry(key, value);
	previous->Next = entry;
	++Size;
	return true;
}

template<typename K, typename V>
V* STable<typename K, typename V>::Get(const K* key)
{
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return nullptr;
	}

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	while (entry)
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			return &entry->Value;
		}
		entry = entry->Next;
	}
	return nullptr;
}

template<typename K, typename V>
bool STable<typename K, typename V>::Contains(const K* key) const
{
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return false;
	}

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	while (entry)
	{
		if (KeyEqualsFunction(key, &entry->Key))
			return true;
		entry = entry->Next;
	}
	return false;
}

template<typename K, typename V>
bool STable<typename K, typename V>::Remove(const K* key)
{
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return false;
	}

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	if (!entry) return false;
	if (!entry->Next)
	{
		FreeEntry(entry);
		Entries[hash] = 0;
		--Size;
		return true;
	}

	STableEntry<K, V>* previous = 0;
	while (entry->Next)
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			FreeEntry(entry);

			if (previous)
				previous->Next = entry->Next;
			else
				Entries[hash] = entry->Next;

			--Size;
			return true;
		}
		previous = entry;
		entry = entry->Next;
	}
	return false;
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
	table.Initialize(5);

	assert(table.Entries);

	Vector2i k = { 1, 2 };
	char v = 1;
	table.Put(&k, &v);

	Vector2i kk = { 2, 2 };
	char vv = 2;
	table.Put(&kk, &vv);

	Vector2i kk1 = { 3, 2 };
	char vv1 = 3;
	table.Put(&kk1, &vv1);

	Vector2i kk2 = { 4, 2 };
	char vv2 = 4;
	table.Put(&kk2, &vv2);

	Vector2i kk3 = { 5, 2 };
	char vv3 = 5;
	table.Put(&kk3, &vv3);

	Vector2i kk4 = { 6, 2 };
	char vv4 = 6;
	table.Put(&kk4, &vv4);

	assert(table.Size == 6);

	Vector2i k1 = { 100, 15 };
	char v1 = 255;
	table.Put(&k1, &v1);

	assert(table.Size == 7);

	Vector2i k2 = { 2, 2 };
	char v2 = 64;
	table.Put(&k2, &v2);

	assert(table.Size == 7);

	char* get = table.Get(&k);
	assert(*get == v);

	bool contains = table.Contains(&k1);

	assert(contains == true);

	bool removed = table.Remove(&kk);

	assert(removed == true);

	Vector2i k3 = { 2, 2 };
	char v3 = 128;
	bool added = table.Put(&k3, &v3);

	assert(added == true);
}
