#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SUtil.h"

#define STABLE_DEFAULT_SPACE 0.8f
#define STABLE_DEFAULT_RESIZE 2

template<typename K>
internal inline uint64_t
STableDefaultKeyHash(const K* key)
{
	const uint8_t* const data = (const uint8_t* const)key;
	size_t length = sizeof(K);
	size_t hash = FNVHash(data, length);
	return hash;
}

template<typename K>
bool
STableDefaultKeyEquals(const K* k0, const K* k1)
{
	return *k0 == *k1;
}

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
	uint32_t Size;
	uint32_t Capacity;
	STableEntry<K, V>** Entries;
	SMemAllocator Allocator;

	uint64_t(*KeyHashFunction)(const K* key) = STableDefaultKeyHash;
	bool (*KeyEqualsFunction)(const K* v0, const K* v1);

	STable() = default;
	STable(bool (*keyEqualsFunction)(const K* v0, const K* v1));

	// Sets Capacity capacity * loadFactor, if the table
	// contains entries will trigger a rehash
	void Reserve(uint32_t estimatedCapacity, float loadFactor); 
	void Free();
	bool Put(const K* key, const V* value);
	V* Get(const K* key) const;
	bool Remove(const K* key);
	bool Contains(const K* key) const;

	inline bool IsAllocated() const;
	inline size_t MemUsed() const;
};

template<typename K, typename V>
STable<K, V>::STable(bool (*keyEqualsFunction)(const K* v0, const K* v1))
	: KeyEqualsFunction(keyEqualsFunction)
{ }

template<typename K, typename V>
internal STableEntry<K, V>*
CreateEntry(const STable<K, V>* table, const K* key, const V* value)
{
	STableEntry<K, V>* entry = (STableEntry<K, V>*)
		table->Allocator.Alloc(sizeof(STableEntry<K, V>));
	entry->Key = *key;
	entry->Value = *value;
	entry->Next = NULL;
	return entry;
}

template<typename K, typename V>
internal inline void
FreeEntry(const STable<K, V>* table, STableEntry<K, V>* entry)
{
	SASSERT(entry);
	table->Allocator.Free(entry);
}

template<typename K, typename V>
internal inline uint64_t
HashKey(const STable<K, V>* sTable, const K* key)
{
	uint64_t hash = sTable->KeyHashFunction(key);
	return hash % sTable->Capacity;
}

template<typename K, typename V>
void STable<K, V>::Reserve(uint32_t capacity, float loadFactor)
{
	SASSERT(loadFactor > 0.0f);
	uint32_t newCap = capacity * loadFactor;
	SASSERT(newCap > Capacity);
	if (!IsAllocated())
	{
		Capacity = newCap;
		Entries = (STableEntry<K, V>**)Allocator.Alloc(MemUsed());
	}
	else if (Size == 0)
	{
		Capacity = newCap;
		Allocator.Free(Entries);
		Entries = (STableEntry<K, V>**)Allocator.Alloc(MemUsed());
	}
	else // Rehash
	{
		SASSERT(Entries);
		// Create temporary table
		STable<K, V> sTable2;
		sTable2.Capacity = newCap;
		sTable2.Size = 0;
		size_t memSize = newCap * sizeof(STableEntry<K, V>*);
		sTable2.Entries = (STableEntry<K, V>**)Allocator.Alloc(memSize);
		SASSERT(sTable2.Entries);
		sTable2.Allocator = Allocator;
		sTable2.KeyHashFunction = KeyHashFunction;
		sTable2.KeyEqualsFunction = KeyEqualsFunction;

		// Put old values, Free old values
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
					FreeEntry(this, next);
					next = tmpNext;
				}
				sTable2.Put(&entry->Key, &entry->Value);
				FreeEntry(this, entry);
			}
		}

		SASSERT(sTable2.Size == Size);

		// Frees old entries
		Allocator.Free(Entries);
		Entries = sTable2.Entries;
		Capacity = sTable2.Capacity;
	}
	SASSERT(Entries);
}

template<typename K, typename V>
void STable<K, V>::Free()
{
	// TODO look at this
	for (uint32_t i = 0; i < Capacity; ++i)
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

	Allocator.Free(Entries);
	Entries = NULL;
	Size = 0;
	Capacity = 0;
}

template<typename K, typename V>
bool STable<K, V>::Put(const K* key, const V* value)
{
	SASSERT(key);
	SASSERT(value);
	SASSERT(KeyEqualsFunction(key, key));

	uint32_t load = (uint32_t)((float)Capacity * STABLE_DEFAULT_SPACE);
	if (Size >= load)
	{
		Reserve(Capacity, STABLE_DEFAULT_RESIZE);
	}

	SASSERT(Entries);
	SASSERT(Capacity > 0);

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	if (!entry) // No entry at hash
	{
		Entries[hash] = CreateEntry(this, key, value);
		++Size;
		return true;
	}

	while (entry) // Loop till end of linked list to insert, break if duplicate
	{
		if (KeyEqualsFunction(key, &entry->Key))
			return false;

		if (!entry->Next)
		{
			entry->Next = CreateEntry(this, key, value);
			++Size;
			break;
		}
		entry = entry->Next;
	}
	return true;
}

template<typename K, typename V>
V* STable<K, V>::Get(const K* key) const
{
	SASSERT(key);
	SASSERT(Entries);
	SASSERT(KeyEqualsFunction(key, key));

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
bool STable<K, V>::Contains(const K* key) const
{
	SASSERT(key);
	SASSERT(Entries);
	SASSERT(KeyEqualsFunction(key, key));

	if (Size == 0) return false;

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
	SASSERT(key);
	SASSERT(Entries);
	SASSERT(KeyEqualsFunction(key, key));

	uint64_t hash = HashKey(this, key);
	STableEntry<K, V>* entry = Entries[hash];
	if (!entry) return false;

	STableEntry<K, V>* last = NULL;
	while (entry)
	{
		if (KeyEqualsFunction(key, &entry->Key))
		{
			if (Entries[hash] == entry) // Set head to next
				Entries[hash] = entry->Next;
			else if (entry->Next && last)
				last->Next = entry->Next;
			FreeEntry(this, entry);
			--Size;
			break;
		}
		if (!entry->Next) return false; // No next not found
		last = entry;
		entry = entry->Next;
	}
	return true;
}

template<typename K, typename V>
inline bool STable<K, V>::IsAllocated() const
{
	return (Entries != nullptr);
}

template<typename K, typename V>
inline size_t STable<K, V>::MemUsed() const
{
	return Capacity * sizeof(STableEntry<K, V>*);
}

#include "Core/Vector2i.h"
inline void TestSTable()
{
	// NOTE: somewhat of a bad test
	// if default resize changes the capacity
	// asserts will trigger

	STable<Vector2i, char> table = {};
	table.KeyEqualsFunction = Vector2iEqualsFunc;
	SASSERT(!table.Entries);
	table.Reserve(2, 1.0f);
	SASSERT(table.IsAllocated());
	SASSERT(table.Capacity == 2);

	Vector2i k = { 1, 2 };
	char v = 1;
	table.Put(&k, &v);

	SASSERT(table.Size == 1);
	SASSERT(table.Capacity == 2);

	Vector2i kk = { 2, 2 };
	char v1 = 2;
	table.Put(&kk, &v1);

	SASSERT(table.Size == 2);
	SASSERT(table.Capacity == 4);

	Vector2i kk1 = { 3, 2 };
	char v2 = 3;
	table.Put(&kk1, &v2);

	Vector2i kk2 = { 4, 2 };
	char v3 = 4;
	table.Put(&kk2, &v3);

	SASSERT(table.Size == 4);
	SASSERT(table.Capacity == 8);

	Vector2i kk3 = { 5, 2 };
	char v4 = 5;
	table.Put(&kk3, &v4);

	Vector2i kk4 = { 6, 2 };
	char v5 = 6;
	table.Put(&kk4, &v5);

	bool added = table.Put(&k, &v);
	SASSERT(!added);

	Vector2i noDup = { 0, 0 };
	bool added3 = table.Put(&noDup, &v);
	SASSERT(added3);

	SASSERT(table.Size == 7);
	SASSERT(table.Capacity == 16);

	char* get = table.Get(&noDup);
	SASSERT(*get == v);
	
	bool contains = table.Contains(&noDup);
	SASSERT(contains == true);

	bool removed = table.Remove(&k);
	SASSERT(removed == true);
	SASSERT(table.Size == 6);

	char* getNull = table.Get(&k);
	SASSERT(getNull == NULL);
}
