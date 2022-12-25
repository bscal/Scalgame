#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define S_TABLE_DEFAULT_CAPACITY 10
#define S_TABLE_DEFAULT_SPACE 0.75f
#define S_TABLE_DEFAULT_RESIZE 2

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

	void Initialize(uint64_t capacity);
	void InitializeEx(uint64_t capacity,
		uint64_t(*KeyHashFunction)(const K* key),
		bool (*KeyEqualsFunction)(const K* v0, const K* v1));
	void Free();
	void Rehash(size_t newCapacity);
	bool Put(const K* key, V* value);
	V* Get(const K* key) const;
	const V& GetRef(const K* key) const;
	bool Remove(const K* key);
	bool Contains(const K* key) const;

	inline bool IsInitialized() const;
};

template<typename K, typename V>
internal STableEntry<K, V>* CreateEntry(const K* key, const V* value)
{
	STableEntry<K, V>* entry = (STableEntry<K, V>*)
		Scal::MemAlloc(sizeof(STableEntry<K, V>));
	entry->Key = *key;
	entry->Value = *value;
	entry->Next = nullptr;
	return entry;
}

template<typename K, typename V>
internal void FreeEntry(STableEntry<K, V>* entry)
{
	Scal::MemFree(entry);
}

template<typename K, typename V>
internal uint64_t HashKey(const STable<K, V>* sTable, const K* key)
{
	uint64_t hash = sTable->KeyHashFunction(key);
	return hash % sTable->Capacity;
}

template<typename K, typename V>
void STable<K, V>::Rehash(size_t newCapacity)
{
	TraceLog(LOG_INFO, "Resizing!");

	STable<K, V> sTable2 = {};
	sTable2.InitializeEx(newCapacity,
		KeyHashFunction, KeyEqualsFunction);

	for (int i = 0; i < Capacity; ++i)
	{
		STableEntry<K, V>* entry = Entries[i];
		if (entry)
		{
			STableEntry<K, V>* next = entry->Next;
			while (next)
			{
				sTable2.Put(&next->Key, &next->Value);
				auto tmpNext = next->Next;
				FreeEntry(next);
				next = tmpNext;
			}
			sTable2.Put(&entry->Key, &entry->Value);
			FreeEntry(entry);
		}
	}
	Scal::MemFree(Entries);
	Entries = sTable2.Entries;
	Size = sTable2.Size;
	Capacity = sTable2.Capacity;
}

template<typename K>
internal uint64_t DefaultHash(const K* key)
{
	return static_cast<uint64_t>(*key);
}

template<typename K>
internal bool DefaultEquals(const K* k0, const K* k1)
{
	return *k0 == *k1;
}

template<typename K, typename V>
void STable<K, V>::Initialize(uint64_t capacity)
{
	if (capacity < 1) capacity = S_TABLE_DEFAULT_CAPACITY;

	if (!KeyHashFunction)
	{
		KeyHashFunction = &DefaultHash;
	}
	if (!KeyEqualsFunction)
	{
		KeyEqualsFunction = &DefaultEquals;
	}

	Size = 0;
	Capacity = capacity;
	Entries = (STableEntry<K, V>**)
		Scal::MemAllocZero(Capacity * sizeof(STableEntry<K, V>*));
}

template<typename K, typename V>
void STable<K, V>::InitializeEx(uint64_t capacity,
	uint64_t(*keyHashFunction)(const K* key),
	bool (*keyEqualsFunction)(const K* v0, const K* v1))
{
	if (capacity < 1) capacity = S_TABLE_DEFAULT_CAPACITY;

	assert(keyHashFunction);
	KeyHashFunction = keyHashFunction;

	assert(keyEqualsFunction);
	KeyEqualsFunction = keyEqualsFunction;

	Size = 0;
	Capacity = capacity;
	Entries = (STableEntry<K, V>**)
		Scal::MemAllocZero(Capacity * sizeof(STableEntry<K, V>*));
}


template<typename K, typename V>
void STable<K, V>::Free()
{
	// TODO look at this
	for (uint64_t i = 0; i < Capacity; ++i)
	{
		STableEntry<K, V>* entry = Entries[i];
		if (!entry) continue;

		auto nextEntry = entry->Next;
		while (nextEntry)
		{
			nextEntry = entry->Next;
			FreeEntry(entry);
			entry = nextEntry;
		}
	}

	Scal::MemFree(Entries);
	Size = 0;
	Capacity = 0;
}

template<typename K, typename V>
bool STable<K, V>::Put(const K* key, V* value)
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

	if ((float)Size >= (float)Capacity * S_TABLE_DEFAULT_SPACE)
	{
		Rehash(Capacity * S_TABLE_DEFAULT_RESIZE);
	}

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	if (!entry) // No entry at hash
	{
		Entries[hash] = CreateEntry(key, value);
		++Size;
		return true;
	}

	while (entry) // Loop till end of linked list to insert, break if duplicate
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			break;
		}
		if (!entry->Next)
		{
			entry->Next = CreateEntry(key, value);
			++Size;
			return true;
		}
		entry = entry->Next;
	}
	return false;
}

template<typename K, typename V>
V* STable<K, V>::Get(const K* key) const
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
const V& STable<K, V>::GetRef(const K* key) const
{
	assert(key);
	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	while (entry)
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			return entry->Value;
		}
		entry = entry->Next;
	}
	return {};
}

template<typename K, typename V>
bool STable<K, V>::Contains(const K* key) const
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
bool STable<K, V>::Remove(const K* key)
{
	if (!key)
	{
		TraceLog(LOG_ERROR, "key cannot be null");
		return false;
	}

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	if (!entry) return false;

	STableEntry<K, V>* previous = 0;
	while (entry)
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			if (entry->Next)
			{
				if (previous)
					previous->Next = entry->Next;
				else
					Entries[hash] = entry->Next;
			}
			else if (previous)
			{
				previous->Next = 0;
			}
			else if (!previous)
			{
				Entries[hash] = 0;
			}
			FreeEntry(entry);
			--Size;
			return true;
		}
		if (!entry->Next)
		{
			break;
		}
		previous = entry;
		entry = entry->Next;
	}
	return false;
}

template<typename K, typename V>
inline bool STable<K, V>::IsInitialized() const
{
	return (Entries != nullptr);
}

inline void TestSTable()
{
	//STable<Vector3, char> table;
	//table.InitializeEx(5, [](const Vector3* key)
	//	{
	//		return Hash
	//	},
	//	[](const Vector3* k0, const Vector3* k1)
	//	{
	//		return *k0 == *k1;
	//	});

	//assert(table.Entries);

	//Vector2i k = { 1, 2 };
	//char v = 1;
	//table.Put(&k, &v);

	//Vector2i kk = { 2, 2 };
	//char vv = 2;
	//table.Put(&kk, &vv);

	//Vector2i kk1 = { 3, 2 };
	//char vv1 = 3;
	//table.Put(&kk1, &vv1);

	//Vector2i kk2 = { 4, 2 };
	//char vv2 = 4;
	//table.Put(&kk2, &vv2);

	//Vector2i kk3 = { 5, 2 };
	//char vv3 = 5;
	//table.Put(&kk3, &vv3);

	//Vector2i kk4 = { 6, 2 };
	//char vv4 = 6;
	//table.Put(&kk4, &vv4);

	//assert(table.Size == 6);

	//Vector2i k1 = { 100, 15 };
	//char v1 = 255;
	//table.Put(&k1, &v1);

	//assert(table.Size == 7);

	//Vector2i k2 = { 2, 2 };
	//char v2 = 64;
	//table.Put(&k2, &v2);

	//assert(table.Size == 7);

	//char* get = table.Get(&k);
	//assert(*get == v);

	//bool contains = table.Contains(&k1);

	//assert(contains == true);

	//bool removed = table.Remove(&kk);

	//assert(removed == true);

	//Vector2i k3 = { 2, 2 };
	//char v3 = 128;
	//bool added = table.Put(&k3, &v3);

	//assert(added == true);
}
