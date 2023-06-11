#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"
#include "Core/SHash.hpp"
#include "Core/SUtil.h"

#define STABLE_DEFAULT_CAPACITY 2
#define STABLE_DEFAULT_LOADFACTOR 0.8f
#define STABLE_DEFAULT_RESIZE 2

template<typename K>
internal inline size_t
STableDefaultKeyHash(const K* key)
{
	const uint8_t* const data = (const uint8_t* const)key;
	size_t length = sizeof(K);
	size_t hash = FNVHash64(data, length);
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
	STableEntry<K, V>** Entries;
	uint32_t Size;
	uint32_t Capacity;
	uint32_t MaxSize;
	SAllocator Allocator;

	// TODO maybe just template these idk
	bool (*KeyEqualsFunction)(const K* v0, const K* v1);
	uint64_t(*KeyHashFunction)(const K* key);

	STable() = default;
	STable(bool (*keyEqualsFunction)(const K* v0, const K* v1),
		uint64_t(*keyHashFunction)(const K* key));

	// Sets Capacity capacity * loadFactor, if the table
	// contains entries will trigger a rehash
	void Reserve(uint32_t estimatedCapacity);
	void ReserveWithLoadFactor(uint32_t estimatedCapacity, float loadFactor);
	void Free();
	void Clear();

	bool Put(const K* key, const V* value);
	V* Get(const K* key) const;
	bool Remove(const K* key);
	bool Contains(const K* key) const;

	inline bool IsAllocated() const { return (Entries != nullptr); }
	inline size_t Stride() const { return sizeof(STableEntry<K, V>); }
	inline size_t MemUsed() const { return Capacity * Stride(); }
};

template<typename K, typename V>
STable<K, V>::STable(bool (*keyEqualsFunction)(const K* v0, const K* v1),
	uint64_t(*keyHashFunction)(const K* key))
	: Entries(nullptr), Allocator(SAllocator::Game),
		Size(0), Capacity(0), MaxSize(0), 
		KeyEqualsFunction(keyEqualsFunction), KeyHashFunction(keyHashFunction)
{
}

template<typename K, typename V>
internal STableEntry<K, V>*
CreateEntry(const STable<K, V>* table, const K* key, const V* value)
{
	size_t entrySize = sizeof(STableEntry<K, V>);
	STableEntry<K, V>* entry = (STableEntry<K, V>*)
		SAlloc(table->Allocator, entrySize, MemoryTag::Tables);
	SASSERT(entry);
	entry->Key = *key;
	entry->Value = *value;
	entry->Next = nullptr;
	return entry;
}

template<typename K, typename V>
internal inline void
FreeEntry(const STable<K, V>* table, STableEntry<K, V>* entry)
{
	SASSERT(entry);
	SFree(table->Allocator, entry, sizeof(STableEntry<K, V>), MemoryTag::Tables);
}

template<typename K, typename V>
internal inline uint64_t
HashKey(const STable<K, V>* sTable, const K* key)
{
	SASSERT(IsPowerOf2(sTable->Capacity));
	SASSERT(sTable->KeyHashFunction);

	uint64_t hash = sTable->KeyHashFunction(key);
	hash &= ((uint64_t)(sTable->Capacity - 1));
	return hash;
}

template<typename K, typename V>
void STable<K, V>::ReserveWithLoadFactor(uint32_t estimatedCapacity, float loadFactor)
{
	uint32_t newCap = (uint32_t)((float)estimatedCapacity * loadFactor);
	if (!IsPowerOf2_32(newCap)) newCap = AlignPowTwo32Ceil(newCap);
	Reserve(newCap);
}

template<typename K, typename V>
void STable<K, V>::Reserve(uint32_t newCapacity)
{
	if (newCapacity == 0) newCapacity = STABLE_DEFAULT_CAPACITY;
	else if (!IsPowerOf2(newCapacity))
	{
		newCapacity = AlignPowTwo32Ceil(newCapacity);
		SLOG_WARN("Reserving STable with non power of 2 capacity!, automatically aligning!");
	}

	SASSERT(IsPowerOf2(newCapacity));
	uint32_t oldCapacity = Capacity; // Used in rehash
	Capacity = newCapacity;
	MaxSize = (uint32_t)((float)Capacity * STABLE_DEFAULT_LOADFACTOR);

	if (!IsAllocated() || Size == 0)
	{
		size_t oldSize = oldCapacity * Stride();
		size_t newSize = newCapacity * Stride();
		Entries = (STableEntry<K, V>**)SRealloc(Allocator, Entries, oldSize, newSize, MemoryTag::Tables);
	}
	else // Rehash
	{
		SASSERT(Entries);
		// Create temporary table
		STable<K, V> sTable2 = {};
		sTable2.Allocator = Allocator;
		sTable2.KeyHashFunction = KeyHashFunction;
		sTable2.KeyEqualsFunction = KeyEqualsFunction;
		sTable2.Reserve(Capacity);

		// Put old values, Free old values
		for (uint32_t i = 0; i < oldCapacity; ++i)
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
		SFree(Allocator, Entries, oldCapacity * Stride(), MemoryTag::Tables);
		*this = sTable2;
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
			FreeEntry(this, entry);
			entry = nextEntry;
		}
	}
	SFree(Allocator, Entries, MemUsed(), MemoryTag::Tables);
	*this = {};
}

template<typename K, typename V>
void STable<K, V>::Clear()
{
	if (!IsTemporaryAllocator(&Allocator))
	{
		for (uint32_t i = 0; i < Capacity; ++i)
		{
			STableEntry<K, V>* entry = Entries[i];
			if (!entry) continue;

			auto nextEntry = entry->Next;
			while (nextEntry)
			{
				nextEntry = entry->Next;
				FreeEntry(this, entry);
				entry = nextEntry;
			}
		}
	}

	SMemClear(Entries, MemAllocated());
	Size = 0;
}

template<typename K, typename V>
bool STable<K, V>::Put(const K* key, const V* value)
{
	SASSERT(key);
	SASSERT(value);
	SASSERT(KeyEqualsFunction(key, key));

	if (Size == MaxSize)
	{
		Reserve(Capacity * STABLE_DEFAULT_RESIZE);
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
			return true;
		}
		entry = entry->Next;
	}
	return false;
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

	STableEntry<K, V>* last = nullptr;
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

#include "Core/Vector2i.h"
inline int TestSTable()
{
	// NOTE: somewhat of a bad test
	// if default resize changes the capacity
	// asserts will trigger

	STable<int, int> testTable = {};
	testTable.Reserve(16);
	SASSERT(testTable.Capacity == 16);

	STable<Vector2i, char> table(STableDefaultKeyEquals, STableDefaultKeyHash);
	SASSERT(!table.Entries);
	table.Reserve(5);
	SASSERT(table.IsAllocated());
	SASSERT(table.Capacity == 8);
	SASSERT(table.MaxSize == (uint32_t)((float)8 * STABLE_DEFAULT_LOADFACTOR));

	Vector2i k = { 1, 2 };
	char v = 1;
	table.Put(&k, &v);

	SASSERT(table.Size == 1);

	Vector2i kk = { 2, 2 };
	char v1 = 2;
	table.Put(&kk, &v1);

	Vector2i kk1 = { 3, 2 };
	char v2 = 3;
	table.Put(&kk1, &v2);

	Vector2i kk2 = { 4, 2 };
	char v3 = 4;
	table.Put(&kk2, &v3);

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
	SASSERT(table.MaxSize == (uint32_t)((float)16 * STABLE_DEFAULT_LOADFACTOR));

	char* get = table.Get(&noDup);
	SASSERT(*get == v);
	
	bool contains = table.Contains(&noDup);
	SASSERT(contains == true);

	bool removed = table.Remove(&k);
	SASSERT(removed == true);
	SASSERT(table.Size == 6);

	char* getNull = table.Get(&k);
	SASSERT(getNull == NULL);

	return 1;
}
